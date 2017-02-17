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
#ifndef __RMQ_MQPUSHCONSUMER_H__
#define __RMQ_MQPUSHCONSUMER_H__

#include <set>
#include <string>

#include "RocketMQClient.h"
#include "MQConsumer.h"
#include "PullResult.h"

namespace rmq
{
	class MessageListener;

	/**
	* 消费者，被动方式消费
	*
	*/
	class MQPushConsumer : public MQConsumer
	{
	public:
		/**
		* 注册消息监听器，一个Consumer只能有一个监听器
		*
		* @param messageListener
		*/
		virtual void registerMessageListener(MessageListener* pMessageListener)=0;

		/**
		* 订阅消息，方法可以调用多次来订阅不同的Topic，也可覆盖之前Topic的订阅过滤表达式
		*
		* @param topic
		*            消息主题
		* @param subExpression
		*            订阅过滤表达式字符串，broker依据此表达式进行过滤。目前只支持或运算<br>
		*            eg: "tag1 || tag2 || tag3"<br>
		*            如果subExpression等于null或者*，则表示全部订阅
		* @param listener
		*            消息回调监听器
		* @throws MQClientException
		*/
		virtual  void subscribe(const std::string& topic, const std::string& subExpression)=0;

		/**
		* 取消订阅，从当前订阅组内注销，消息会被订阅组内其他订阅者订阅
		*
		* @param topic
		*            消息主题
		*/
		virtual void unsubscribe(const std::string& topic)=0;

		/**
		* 动态调整消费线程池线程数量
		*
		* @param corePoolSize
		*/
		virtual void updateCorePoolSize(int corePoolSize)=0;

		/**
		* 消费线程挂起，暂停消费
		*/
		virtual void suspend()=0;

		/**
		* 消费线程恢复，继续消费
		*/
		virtual void resume()=0;
	};
}
#endif
