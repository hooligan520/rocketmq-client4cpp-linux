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
#ifndef __ALLOCATEMESSAGEQUEUESTRATEGYINNER_H__
#define __ALLOCATEMESSAGEQUEUESTRATEGYINNER_H__

#include <algorithm>

#include "AllocateMessageQueueStrategy.h"
#include "MQClientException.h"
#include "UtilAll.h"


namespace rmq
{
    /**
    * Consumer队列自动分配策略,平均分配队列算法(按页分配)
    *
    */
    class AllocateMessageQueueAveragely : public AllocateMessageQueueStrategy
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
        virtual ~AllocateMessageQueueAveragely() {}
        virtual std::vector<MessageQueue>* allocate(
				const std::string& consumerGroup,
				const std::string& currentCID,
                std::vector<MessageQueue>& mqAll,
                std::list<std::string>& cidAll)
        {
            if (currentCID.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "currentCID is empty", -1);
            }

            if (mqAll.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "mqAll is empty", -1);
            }

            if (cidAll.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "cidAll is empty", -1);
            }

            int index = -1;
            int cidAllSize = cidAll.size();

            std::list<std::string>::iterator it = cidAll.begin();
            for (int i = 0; it != cidAll.end(); it++, i++)
            {
                if (*it == currentCID)
                {
                    index = i;
                    break;
                }
            }

            // 不存在此ConsumerId ,直接返回
            if (index == -1)
            {
				RMQ_ERROR("[BUG] ConsumerGroup: {%s} The consumerId: {%s} not in cidAll: {%s}", //
                    consumerGroup.c_str(),
                    currentCID.c_str(),
                    UtilAll::toString(cidAll).c_str());
                return NULL;
            }

            int mqAllSize = mqAll.size();
            int mod = mqAllSize % cidAllSize;
            int averageSize =
                mqAllSize <= cidAllSize ? 1 : (mod > 0 && index < mod ? mqAllSize / cidAllSize
                                               + 1 : mqAllSize / cidAllSize);
            int startIndex = (mod > 0 && index < mod) ? index * averageSize : index * averageSize + mod;

            std::vector<MessageQueue>* result = new std::vector<MessageQueue>();
            int range = std::min<int>(averageSize, mqAllSize - startIndex);

            for (int i = 0; i < range; i++)
            {
                result->push_back(mqAll.at((startIndex + i) % mqAllSize));
            }

            return result;
        }

        virtual std::string getName()
        {
            return "AVG";
        }
    };


	/**
    * Consumer队列自动分配策略,平均分配队列算法(环形分配)
    *
    */
    class AllocateMessageQueueAveragelyByCircle : public AllocateMessageQueueStrategy
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
        virtual ~AllocateMessageQueueAveragelyByCircle() {}
        virtual std::vector<MessageQueue>* allocate(
				const std::string& consumerGroup,
				const std::string& currentCID,
                std::vector<MessageQueue>& mqAll,
                std::list<std::string>& cidAll)
        {
            if (currentCID.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "currentCID is empty", -1);
            }

            if (mqAll.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "mqAll is empty", -1);
            }

            if (cidAll.empty())
            {
                THROW_MQEXCEPTION(MQClientException, "cidAll is empty", -1);
            }

            int index = -1;
            std::list<std::string>::iterator it = cidAll.begin();
            for (int i = 0; it != cidAll.end(); it++, i++)
            {
                if (*it == currentCID)
                {
                    index = i;
                    break;
                }
            }

            // 不存在此ConsumerId ,直接返回
            if (index == -1)
            {
				RMQ_ERROR("[BUG] ConsumerGroup: {%s} The consumerId: {%s} not in cidAll: {%s}", //
                    consumerGroup.c_str(),
                    currentCID.c_str(),
                    UtilAll::toString(cidAll).c_str());
                return NULL;
            }

			std::vector<MessageQueue>* result = new std::vector<MessageQueue>();
	        for (int i = index; i < (int)mqAll.size(); i++)
			{
	            if (i % (int)cidAll.size() == index)
				{
					result->push_back(mqAll.at(i));
	            }
	        }

	        return result;
        }

        virtual std::string getName()
        {
            return "AVG_BY_CIRCLE";
        }
    };

    /**
    * Consumer队列自动分配策略,按照配置来分配队列，建议应用使用Spring来初始化
    *
    */
    class AllocateMessageQueueByConfig : public AllocateMessageQueueStrategy
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
        virtual ~AllocateMessageQueueByConfig() {}
        virtual std::vector<MessageQueue>* allocate(
				const std::string& consumerGroup,
				const std::string& currentCID,
                std::vector<MessageQueue>& mqAll,
                std::list<std::string>& cidAll)
        {
            return NULL;
        }

        virtual std::string getName()
        {
            return "CONFIG";
        }
    };

    /**
    * Consumer队列自动分配策略,按照机房来分配队列，例如支付宝逻辑机房
    *
    */
    class AllocateMessageQueueByMachineRoom : public AllocateMessageQueueStrategy
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
        virtual ~AllocateMessageQueueByMachineRoom() {}
        virtual std::vector<MessageQueue>* allocate(
				const std::string& consumerGroup,
				const std::string& currentCID,
                std::vector<MessageQueue>& mqAll,
                std::list<std::string>& cidAll)
        {
            return NULL;
        }

        virtual std::string getName()
        {
            return "MACHINE_ROOM";
        }
    };
}

#endif
