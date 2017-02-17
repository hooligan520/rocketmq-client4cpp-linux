/**
* Copyright (C) 2013 kangliqiang ,kangliq@163.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __RMQ_MESSAGELISTENER_H__
#define __RMQ_MESSAGELISTENER_H__

#include <limits.h>
#include <list>

#include "MessageExt.h"
#include "MessageQueue.h"

namespace rmq
{
	/**
	* 消息监听器，被动方式订阅消息使用，需要用户实现<br>
	* 应用不可以直接继承此接口
	*
	*/
	class MessageListener
	{
	public:
		virtual ~MessageListener(){}
	};

	enum ConsumeOrderlyStatus
	{
		SUCCESS,// 消息处理成功
		ROLLBACK,// 回滚消息
		COMMIT,// 提交消息
		SUSPEND_CURRENT_QUEUE_A_MOMENT,// 将当前队列挂起一小会儿
	};

	typedef struct tagConsumeOrderlyContext
	{
		tagConsumeOrderlyContext(MessageQueue& mq)
			:messageQueue(mq),
			autoCommit(true),
			suspendCurrentQueueTimeMillis(1000)
		{

		}

		MessageQueue messageQueue;///< 要消费的消息属于哪个队列
		bool autoCommit;///< 消息Offset是否自动提交
		long suspendCurrentQueueTimeMillis;
	}ConsumeOrderlyContext;

	class MessageListenerOrderly : public MessageListener
	{
		/**
		* 方法抛出异常等同于返回 ConsumeOrderlyStatus.SUSPEND_CURRENT_QUEUE_A_MOMENT<br>
		* P.S: 建议应用不要抛出异常
		*
		* @param msgs
		*            msgs.size() >= 1<br>
		*            DefaultMQPushConsumer.consumeMessageBatchMaxSize=1，默认消息数为1
		* @param context
		* @return
		*/
	public:
		virtual ConsumeOrderlyStatus consumeMessage(std::list<MessageExt*>& msgs,
													ConsumeOrderlyContext& context)=0;
	};

	enum ConsumeConcurrentlyStatus
	{
		CONSUME_SUCCESS,// 表示消费成功
		RECONSUME_LATER,// 表示消费失败，但是稍后还会重新消费这批消息
	};

	struct ConsumeConcurrentlyContext
	{
		ConsumeConcurrentlyContext(MessageQueue& mq)
			:messageQueue(mq),
			delayLevelWhenNextConsume(0),
			ackIndex(INT_MAX)
		{
		}
		MessageQueue messageQueue;///< 要消费的消息属于哪个队列
		/**
		* 下次消息重试延时时间<br>
		* -1，表示不重试，直接进入死信队列<br>
		* 0，表示由服务器根据重试次数自动叠加<br>
		* >0，表示客户端强制指定延时Level
		*/
		int delayLevelWhenNextConsume;
		int ackIndex;///< 对于批量消费，ack至哪条消息，默认全部ack，至最后一条消息
	};

	class MessageListenerConcurrently : public MessageListener
	{
	public:
		/**
		* 方法抛出异常等同于返回 ConsumeConcurrentlyStatus.RECONSUME_LATER<br>
		* P.S: 建议应用不要抛出异常
		*
		* @param msgs
		*            msgs.size() >= 1<br>
		*            DefaultMQPushConsumer.consumeMessageBatchMaxSize=1，默认消息数为1
		* @param context
		* @return
		*/
		virtual ConsumeConcurrentlyStatus consumeMessage(std::list<MessageExt*>& msgs,
														ConsumeConcurrentlyContext& context)=0;
	};
}

#endif
