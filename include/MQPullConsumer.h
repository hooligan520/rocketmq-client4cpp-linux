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
#ifndef __RMQ_MQPULLCONSUMER_H__
#define __RMQ_MQPULLCONSUMER_H__

#include <set>
#include <string>

#include "RocketMQClient.h"
#include "MQConsumer.h"
#include "PullResult.h"

namespace rmq
{
	class MessageQueueListener;
	class MessageQueue;
	class PullCallback;

	/**
	* 消费者，主动方式消费
	*
	*/
	class MQPullConsumer : public MQConsumer
	{
	public:
		virtual ~MQPullConsumer(){}
		/**
		* 注册监听队列变化的listener对象
		*
		* @param topic
		* @param listener
		*            一旦发生变化，客户端会主动回调listener对象
		*/
		virtual void registerMessageQueueListener(const std::string& topic, MessageQueueListener* pListener)=0;

		/**
		* 指定队列，主动拉取消息，即使没有消息，也立刻返回
		*
		* @param mq
		*            指定具体要拉取的队列
		* @param subExpression
		*            订阅过滤表达式字符串，broker依据此表达式进行过滤。目前只支持或运算<br>
		*            eg: "tag1 || tag2 || tag3"<br>
		*            如果subExpression等于null或者*，则表示全部订阅
		* @param offset
		*            从指定队列哪个位置开始拉取
		* @param maxNums
		*            一次最多拉取条数
		* @return 参见PullResult
		* @throws MQClientException
		* @throws InterruptedException
		* @throws MQBrokerException
		* @throws RemotingException
		*/
		virtual PullResult* pull(MessageQueue& mq,
			const std::string& subExpression,
			long long offset,int maxNums)=0;

		virtual void pull(MessageQueue& mq,
			const std::string& subExpression,
			long long offset,
			int maxNums,
			PullCallback* pPullCallback)=0;

		/**
		* 指定队列，主动拉取消息，如果没有消息，则broker阻塞一段时间再返回（时间可配置）<br>
		* broker阻塞期间，如果有消息，则立刻将消息返回
		*
		* @param mq
		*            指定具体要拉取的队列
		* @param subExpression
		*            订阅过滤表达式字符串，broker依据此表达式进行过滤。目前只支持或运算<br>
		*            eg: "tag1 || tag2 || tag3"<br>
		*            如果subExpression等于null或者*，则表示全部订阅
		* @param offset
		*            从指定队列哪个位置开始拉取
		* @param maxNums
		*            一次最多拉取条数
		* @return 参见PullResult
		* @throws InterruptedException
		* @throws MQBrokerException
		* @throws RemotingException
		* @throws MQClientException
		*/
		virtual PullResult* pullBlockIfNotFound(MessageQueue& mq,
												const std::string& subExpression,
												long long offset,
												int maxNums)=0;


		virtual void pullBlockIfNotFound(MessageQueue& mq,
										 const std::string& subExpression,
										 long long offset,
										 int maxNums,
										 PullCallback* pPullCallback)=0;

		/**
		* 更新消费进度<br>
		* 只是更新Consumer缓存中的数据，如果是广播模式，则定时更新到本地存储<br>
		* 如果是集群模式，则定时更新到远端Broker<br>
		* <p/>
		* P.S. 可频繁调用，无性能开销
		*
		* @param mq
		* @param offset
		* @throws MQClientException
		*/
		virtual void updateConsumeOffset(MessageQueue& mq, long long offset)=0;

		/**
		* 获取消费进度，返回-1表示出错
		*
		* @param mq
		* @param fromStore
		* @return
		* @throws MQClientException
		*/
		virtual long long fetchConsumeOffset(MessageQueue& mq, bool fromStore)=0;

		/**
		* 根据topic获取MessageQueue，以均衡方式在组内多个成员之间分配
		*
		* @param topic
		*            消息Topic
		* @return 返回队列集合
		* @throws MQClientException
		*/
		virtual std::set<MessageQueue>* fetchMessageQueuesInBalance(const std::string& topic)=0;
	};
}
#endif
