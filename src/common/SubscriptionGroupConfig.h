/**
* Copyright (C) 2013 kangliqiang, kangliq@163.com
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
#ifndef __SUBSCRIPTIONGROUPCONFIG_H__
#define __SUBSCRIPTIONGROUPCONFIG_H__

#include <string>
#include "MixAll.h"

namespace rmq
{
    class SubscriptionGroupConfig
    {
    public:
        SubscriptionGroupConfig(const std::string& groupName)
        {
            this->groupName = groupName;
            consumeEnable = true;
            consumeFromMinEnable = true;
            consumeBroadcastEnable = true;
            retryQueueNums = 1;
            retryMaxTimes = 5;
            brokerId = MixAll::MASTER_ID;
            whichBrokerWhenConsumeSlowly = 1;
        }

        std::string groupName;// 订阅组名
        bool consumeEnable;// 消费功能是否开启
        bool consumeFromMinEnable;// 是否允许从队列最小位置开始消费，线上默认会设置为false
        bool consumeBroadcastEnable;// 是否允许广播方式消费
        int retryQueueNums;// 消费失败的消息放到一个重试队列，每个订阅组配置几个重试队列
        int retryMaxTimes; // 重试消费最大次数，超过则投递到死信队列，不再投递，并报警
        long brokerId;// 从哪个Broker开始消费
        long whichBrokerWhenConsumeSlowly;// 发现消息堆积后，将Consumer的消费请求重定向到另外一台Slave机器
    };
}

#endif
