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

#ifndef __RMQ_MQADMIN_H__
#define __RMQ_MQADMIN_H__

#include <string>

#include "RocketMQClient.h"
#include "MessageExt.h"

namespace rmq
{
	class MQClientException;
	class RemotingException;
	class MQBrokerException;
	class InterruptedException;
	class MessageQueue;
	class QueryResult;

	/**
	* MQ管理类接口
	*
	*/
	class MQAdmin
	{
	public:
		MQAdmin()
		{
		}

		virtual ~MQAdmin()
		{
		}

		/**
		* 创建topic
		*
		* @param key
		*            请向运维人员申请
		* @param newTopic
		*            要创建的新topic
		* @param queueNum
		*            新topic队列数
		* @throws MQClientException
		*/
		virtual void createTopic(const std::string& key, const std::string& newTopic, int queueNum)=0;

		/**
		* 根据时间查询对应的offset，精确到毫秒<br>
		* P.S. 当前接口有较多IO开销，请勿频繁调用
		*
		* @param mq
		*            队列
		* @param timestamp
		*            毫秒形式时间戳
		* @return 指定时间对应的offset
		* @throws MQClientException
		*/
		virtual long long searchOffset(const MessageQueue& mq, long long timestamp)=0;

		/**
		* 向服务器查询队列最大Offset PS: 最大Offset无对应消息，减1有消息
		*
		* @param mq
		*            队列
		* @return 队列的最大Offset
		* @throws MQClientException
		*/
		virtual long long maxOffset(const MessageQueue& mq)=0;

		/**
		* 向服务器查询队列最小Offset PS: 最小Offset有对应消息
		*
		* @param mq
		*            队列
		* @return 队列的最小Offset
		* @throws MQClientException
		*/
		virtual long long minOffset(const MessageQueue& mq)=0;

		/**
		* 向服务器查询队列保存的最早消息对应的存储时间
		*
		* @param mq
		*            队列
		* @return 最早消息对应的存储时间，精确到毫秒
		* @throws MQClientException
		*/
		virtual long long earliestMsgStoreTime(const MessageQueue& mq)=0;

		/**
		* 根据消息ID，从服务器获取完整的消息
		*
		* @param msgId
		* @return 完整消息
		* @throws InterruptedException
		* @throws MQBrokerException
		* @throws RemotingException
		* @throws MQClientException
		*/
		virtual MessageExt* viewMessage(const std::string& msgId)=0;

		/**
		* 根据消息Key查询消息
		*
		* @param topic
		*            消息主题
		* @param key
		*            消息关键词
		* @param maxNum
		*            查询最大条数
		* @param begin
		*            起始时间戳
		* @param end
		*            结束时间戳
		* @return 查询结果
		* @throws MQClientException
		* @throws InterruptedException
		*/
		virtual QueryResult queryMessage(const std::string& topic,
										 const std::string&  key,
										 int maxNum,
										 long long begin,
										 long long end)=0;
	};
}

#endif
