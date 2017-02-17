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

#ifndef __RMQ_DEFAULTMQPULLCONSUMER_H__
#define __RMQ_DEFAULTMQPULLCONSUMER_H__

#include <list>
#include <string>

#include "RocketMQClient.h"
#include "MQClientException.h"
#include "MessageQueue.h"
#include "MessageExt.h"
#include "ClientConfig.h"
#include "MQPullConsumer.h"

namespace rmq
{
	class OffsetStore;
	class DefaultMQPullConsumerImpl;
	class AllocateMessageQueueStrategy;

	/**
	* 消费者，主动拉取方式消费
	*
	*/
	class DefaultMQPullConsumer : public ClientConfig , public MQPullConsumer
	{
	public:
		DefaultMQPullConsumer();
		DefaultMQPullConsumer(const std::string& consumerGroup);
		~DefaultMQPullConsumer();

		//MQAdmin
		void createTopic(const std::string& key, const std::string& newTopic, int queueNum);
		long long searchOffset(const MessageQueue& mq, long long timestamp);
		long long maxOffset(const MessageQueue& mq);
		long long minOffset(const MessageQueue& mq);
		long long earliestMsgStoreTime(const MessageQueue& mq);
		MessageExt* viewMessage(const std::string& msgId);
		QueryResult queryMessage(const std::string& topic,
								 const std::string&  key,
								 int maxNum,
								 long long begin,
								 long long end);
		// MQadmin end

		AllocateMessageQueueStrategy* getAllocateMessageQueueStrategy();
		void setAllocateMessageQueueStrategy(AllocateMessageQueueStrategy* pAllocateMessageQueueStrategy);
		int getBrokerSuspendMaxTimeMillis() ;
		void setBrokerSuspendMaxTimeMillis(int brokerSuspendMaxTimeMillis);
		std::string getConsumerGroup();
		void setConsumerGroup(const std::string& consumerGroup);
		int getConsumerPullTimeoutMillis();
		void setConsumerPullTimeoutMillis(int consumerPullTimeoutMillis);
		int getConsumerTimeoutMillisWhenSuspend() ;
		void setConsumerTimeoutMillisWhenSuspend(int consumerTimeoutMillisWhenSuspend);
		MessageModel getMessageModel();
		void setMessageModel(MessageModel messageModel);
		MessageQueueListener* getMessageQueueListener();
		void setMessageQueueListener(MessageQueueListener* pMessageQueueListener);
		std::set<std::string> getRegisterTopics();
		void setRegisterTopics( std::set<std::string> registerTopics);

		//MQConsumer
		void sendMessageBack(MessageExt& msg, int delayLevel);
		void sendMessageBack(MessageExt& msg, int delayLevel, const std::string& brokerName);
		std::set<MessageQueue>* fetchSubscribeMessageQueues(const std::string& topic);
		void start();
		void shutdown() ;
		//MQConsumer end

		//MQPullConsumer
		void registerMessageQueueListener(const std::string& topic, MessageQueueListener* pListener);
		PullResult* pull(MessageQueue& mq, const std::string& subExpression, long long offset,int maxNums);
		void pull(MessageQueue& mq,
			const std::string& subExpression,
			long long offset,
			int maxNums,
			PullCallback* pPullCallback);

		PullResult* pullBlockIfNotFound(MessageQueue& mq,
			const std::string& subExpression,
			long long offset,
			int maxNums);

		void pullBlockIfNotFound(MessageQueue& mq,
								 const std::string& subExpression,
								 long long offset,
								 int maxNums,
								 PullCallback* pPullCallback);

		void updateConsumeOffset(MessageQueue& mq, long long offset);

		long long fetchConsumeOffset(MessageQueue& mq, bool fromStore);

		std::set<MessageQueue>* fetchMessageQueuesInBalance(const std::string& topic);
		//MQPullConsumer end

		OffsetStore* getOffsetStore();
		void setOffsetStore(OffsetStore* offsetStore);

		DefaultMQPullConsumerImpl* getDefaultMQPullConsumerImpl();

	protected:
		DefaultMQPullConsumerImpl* m_pDefaultMQPullConsumerImpl;

	private:
		/**
		* 做同样事情的Consumer归为同一个Group，应用必须设置，并保证命名唯一
		*/
		std::string m_consumerGroup;

		/**
		* 长轮询模式，Consumer连接在Broker挂起最长时间，不建议修改
		*/
		int m_brokerSuspendMaxTimeMillis ;

		/**
		* 长轮询模式，Consumer超时时间（必须要大于brokerSuspendMaxTimeMillis），不建议修改
		*/
		int m_consumerTimeoutMillisWhenSuspend;

		/**
		* 非阻塞拉模式，Consumer超时时间，不建议修改
		*/
		int m_consumerPullTimeoutMillis;

		/**
		* 集群消费/广播消费
		*/
		MessageModel m_messageModel;

		/**
		* 队列变化监听器
		*/
		MessageQueueListener* m_pMessageQueueListener;

		/**
		* Offset存储，系统会根据客户端配置自动创建相应的实现，如果应用配置了，则以应用配置的为主
		*/
		OffsetStore* m_pOffsetStore;

		/**
		* 需要监听哪些Topic的队列变化
		*/
		std::set<std::string> m_registerTopics;

		/**
		* 队列分配算法，应用可重写
		*/
		AllocateMessageQueueStrategy* m_pAllocateMessageQueueStrategy;
	};
}

#endif
