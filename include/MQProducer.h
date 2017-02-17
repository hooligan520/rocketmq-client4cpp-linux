/**
* Copyright (C) 2013 kangliqiang ,kangliq@163.com
*
* Licensed under the Apache License, Version 2.0 (the "License")=0;
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

#ifndef __RMQ_MQPRODUCER_H__
#define __RMQ_MQPRODUCER_H__

#include <vector>
#include <string>

#include "RocketMQClient.h"
#include "MQAdmin.h"
#include "SendResult.h"

namespace rmq
{
	class MessageQueue;
	class SendCallback;
	class LocalTransactionExecuter;
	class MessageQueueSelector;

	/**
	* 消息生产者
	*
	*/
	class MQProducer : public MQAdmin
	{
	public:
		MQProducer()
		{

		}

		virtual ~MQProducer()
		{

		}

		/**
		* 启动服务
		*
		* @throw( MQClientException
		*/
		virtual void start()=0;

		/**
		* 关闭服务，一旦关闭，此对象将不可用
		*/
		virtual void shutdown()=0;

		/**
		* 根据topic获取对应的MessageQueue，如果是顺序消息，则按照顺序消息配置返回
		*
		* @param topic
		*            消息Topic
		* @return 返回队列集合
		* @throw( MQClientException
		*/
		virtual std::vector<MessageQueue>* fetchPublishMessageQueues(const std::string& topic)=0;

		/**
		* 发送消息，同步调用
		*
		* @param msg
		*            消息
		* @return 发送结果
		* @throw( InterruptedException
		* @throw( MQBrokerException
		* @throw( RemotingException
		* @throw( MQClientException
		*/
		virtual SendResult send(Message& msg)=0;

		/**
		* 发送消息，异步调用
		*
		* @param msg
		*            消息
		* @param sendCallback
		*            发送结果通过此接口回调
		* @throw( MQClientException
		* @throw( RemotingException
		* @throw( InterruptedException
		*/
		virtual void send(Message& msg, SendCallback* sendCallback)=0;

		/**
		* 发送消息，Oneway形式，服务器不应答，无法保证消息是否成功到达服务器
		*
		* @param msg
		*            消息
		* @throw( MQClientException
		* @throw( RemotingException
		* @throw( InterruptedException
		*/
		virtual void sendOneway(Message& msg)=0;

		/**
		* 向指定队列发送消息，同步调用
		*
		* @param msg
		*            消息
		* @param mq
		*            队列
		* @return 发送结果
		* @throw( InterruptedException
		* @throw( MQBrokerException
		* @throw( RemotingException
		* @throw( MQClientException
		*/
		virtual SendResult send(Message& msg, MessageQueue& mq)=0;

		/**
		* 向指定队列发送消息，异步调用
		*
		* @param msg
		*            消息
		* @param mq
		*            队列
		* @param sendCallback
		*            发送结果通过此接口回调
		* @throw( InterruptedException
		* @throw( RemotingException
		* @throw( MQClientException
		*/
		virtual void send(Message& msg, MessageQueue& mq, SendCallback* sendCallback)=0;

		/**
		* 向指定队列发送消息，Oneway形式，服务器不应答，无法保证消息是否成功到达服务器
		*
		* @param msg
		*            消息
		* @param mq
		*            队列
		* @throw( MQClientException
		* @throw( RemotingException
		* @throw( InterruptedException
		*/
		virtual void sendOneway(Message& msg, MessageQueue& mq)=0;

		/**
		* 发送消息，可以自定义选择队列，队列的总数可能会由于Broker的启停变化<br>
		* 如果要保证消息严格有序，在向运维人员申请Topic时，需要特别说明<br>
		* 同步调用
		*
		* @param msg
		*            消息
		* @param selector
		*            队列选择器，发送时会回调
		* @param arg
		*            回调队列选择器时，此参数会传入队列选择方法
		* @return 发送结果
		* @throw( InterruptedException
		* @throw( MQBrokerException
		* @throw( RemotingException
		* @throw( MQClientException
		*/
		virtual SendResult send(Message& msg, MessageQueueSelector* selector, void* arg)=0;

		/**
		* 发送消息，可以自定义选择队列，队列的总数可能会由于Broker的启停变化<br>
		* 如果要保证消息严格有序，在向运维人员申请Topic时，需要特别说明<br>
		* 异步调用
		*
		* @param msg
		*            消息
		* @param selector
		*            队列选择器，发送时会回调
		* @param arg
		*            回调队列选择器时，此参数会传入队列选择方法
		* @param sendCallback
		*            发送结果通过此接口回调
		* @throw( MQClientException
		* @throw( RemotingException
		* @throw( InterruptedException
		*/
		virtual void send(Message& msg, MessageQueueSelector* selector, void* arg, SendCallback* sendCallback)=0;

		/**
		* 发送消息，可以自定义选择队列，队列的总数可能会由于Broker的启停变化<br>
		* 如果要保证消息严格有序，在向运维人员申请Topic时，需要特别说明<br>
		* Oneway形式，服务器不应答，无法保证消息是否成功到达服务器
		*
		* @param msg
		*            消息
		* @param selector
		*            队列选择器，发送时会回调
		* @param arg
		*            回调队列选择器时，此参数会传入队列选择方法
		* @throw( MQClientException
		* @throw( RemotingException
		* @throw( InterruptedException
		*/
		virtual void sendOneway(Message& msg, MessageQueueSelector* selector, void* arg)=0;

		virtual TransactionSendResult sendMessageInTransaction(Message& msg,
																LocalTransactionExecuter* tranExecuter,
																void* arg)=0;
	};
}
#endif
