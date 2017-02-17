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
#ifndef __RMQ_ALLOCATEMESSAGEQUEUESTRATEGY_H__
#define __RMQ_ALLOCATEMESSAGEQUEUESTRATEGY_H__

#include <vector>
#include <string>
#include <list>

#include "RocketMQClient.h"
#include "MessageQueue.h"

namespace rmq
{
	/**
	* Consumer队列自动分配策略
	*
	*/
	class AllocateMessageQueueStrategy
	{
		/**
		* 给当前的ConsumerId分配队列
		*
		* @param currentCID
		*            当前ConsumerId
		* @param mqAll
		*            当前Topic的所有队列集合，无重复数据，且有序
		* @param cidAll
		*            当前订阅组的所有Consumer集合，无重复数据，且有序
		* @return 分配结果，无重复数据
		*/
	public:
		virtual ~AllocateMessageQueueStrategy() {}
		virtual std::vector<MessageQueue>* allocate(
				const std::string& consumerGroup,
				const std::string& currentCID,
				std::vector<MessageQueue>& mqAll,
				std::list<std::string>& cidAll)=0;
		virtual std::string getName()=0;
	};
}

#endif
