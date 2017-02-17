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
#ifndef __CONSUMERSTAT_H__
#define __CONSUMERSTAT_H__

#include <list>
#include <string>

#include "AtomicValue.h"
#include "KPRUtil.h"
#include "Mutex.h"
#include "ScopedLock.h"

namespace rmq
{
    /**
    * Consumer内部运行时统计信息
    *
    */
    struct ConsumerStat
    {
        long long createTimestamp;// 打点时间戳
        kpr::AtomicLong consumeMsgRTMax;// 一次消费消息的最大RT
        kpr::AtomicLong consumeMsgRTTotal;// 每次消费消息RT叠加总和
        kpr::AtomicLong consumeMsgOKTotal;// 消费消息成功次数总和
        kpr::AtomicLong consumeMsgFailedTotal;// 消费消息失败次数总和
        kpr::AtomicLong pullRTTotal;// 拉消息RT叠加总和（只包含成功拉到的）
        kpr::AtomicLong pullTimesTotal;// 拉消息次数（只包含成功拉到的）

		ConsumerStat()
		{
			createTimestamp = KPRUtil::GetCurrentTimeMillis();
			consumeMsgRTMax = 0;
			consumeMsgRTTotal = 0;
			consumeMsgOKTotal = 0;
			consumeMsgFailedTotal = 0;
			pullRTTotal = 0;
			pullTimesTotal = 0;
		}
    };

    /**
    * 用来统计Consumer运行状态
    *
    */
    class ConsumerStatManager
    {
    public:
        ConsumerStat& getConsumertat()
        {
            return m_consumertat;
        }

        std::list<ConsumerStat>& getSnapshotList()
        {
            return m_snapshotList;
        }

        /**
        * 每隔1秒记录一次
        */
        void recordSnapshotPeriodically()
        {
            kpr::ScopedWLock<kpr::RWMutex> lock(m_snapshotListLock);
            m_snapshotList.push_back(m_consumertat);
            if (m_snapshotList.size() > 60)
            {
                m_snapshotList.pop_front();
            }
        }

        /**
        * 每隔1分钟记录一次
        */
        void logStatsPeriodically(std::string& group, std::string& clientId)
        {
            kpr::ScopedRLock<kpr::RWMutex> lock(m_snapshotListLock);
            if (m_snapshotList.size() >= 60)
            {
                ConsumerStat& first = m_snapshotList.front();
                ConsumerStat& last = m_snapshotList.back();

                // 消费情况
                {
                    double avgRT = (last.consumeMsgRTTotal.get() - first.consumeMsgRTTotal.get())
                                   /
                                   (double)((last.consumeMsgOKTotal.get() + last.consumeMsgFailedTotal.get())
                                            - (first.consumeMsgOKTotal.get() + first.consumeMsgFailedTotal.get()));

                    double tps = ((last.consumeMsgOKTotal.get() + last.consumeMsgFailedTotal.get())
                                  - (first.consumeMsgOKTotal.get() + first.consumeMsgFailedTotal.get()))
                                 / (double)(last.createTimestamp - first.createTimestamp);

                    tps *= 1000;

                    RMQ_INFO(
                        "Consumer, {%s} {%s}, ConsumeAvgRT: {%f} ConsumeMaxRT: {%lld} TotalOKMsg: {%lld} TotalFailedMsg: {%lld} consumeTPS: {%f}",
                        group.c_str(),
                        clientId.c_str(),
                        avgRT,
                        last.consumeMsgRTMax.get(),
                        last.consumeMsgOKTotal.get(),
                        last.consumeMsgFailedTotal.get(),
                        tps);
                }

                // 拉消息情况
                {
                    double avgRT = (last.pullRTTotal.get() - first.pullRTTotal.get())
                                   / (double)(last.pullTimesTotal.get() - first.pullTimesTotal.get());

                    RMQ_INFO("Consumer, {%s} {%s}, PullAvgRT: {%f}  PullTimesTotal: {%lld}",
                             group.c_str(),
                             clientId.c_str(),
                             avgRT,
                             last.pullTimesTotal.get());
                }
            }
        }

    private:
        ConsumerStat m_consumertat;
        std::list<ConsumerStat> m_snapshotList;
        kpr::RWMutex m_snapshotListLock;
    };
}

#endif
