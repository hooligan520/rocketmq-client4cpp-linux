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

#include "ProcessQueue.h"
#include "MessageExt.h"
#include "KPRUtil.h"
#include "ScopedLock.h"

namespace rmq
{

// 客户端本地Lock存活最大时间，超过则自动过期，单位ms
//"rocketmq.client.rebalance.lockMaxLiveTime", "30000"
unsigned int ProcessQueue::s_RebalanceLockMaxLiveTime = 30000;

// 定时Lock间隔时间，单位ms
//"rocketmq.client.rebalance.lockInterval", "20000"
unsigned int ProcessQueue::s_RebalanceLockInterval = 20000;

// 最大拉取idle时间，单位ms
// rocketmq.client.pull.pullMaxIdleTime, 120000
unsigned int ProcessQueue::s_PullMaxIdleTime = 120000;


ProcessQueue::ProcessQueue()
{
	m_lastPullTimestamp = KPRUtil::GetCurrentTimeMillis();
	m_lastConsumeTimestamp = KPRUtil::GetCurrentTimeMillis();
    m_queueOffsetMax = 0L;
    m_msgCount = 0;
    m_dropped = false;

    m_locked = false;
    m_lastLockTimestamp = KPRUtil::GetCurrentTimeMillis();
    m_consuming = false;

}

bool ProcessQueue::isLockExpired()
{
    bool result = (KPRUtil::GetCurrentTimeMillis() - m_lastLockTimestamp) > s_RebalanceLockMaxLiveTime;
    return result;
}

bool ProcessQueue::putMessage(const std::list<MessageExt*>& msgs)
{
    bool dispathToConsume = false;
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);

        int validMsgCnt = 0;
        std::list<MessageExt*>::const_iterator it = msgs.begin();
        for (; it != msgs.end(); it++)
        {
            MessageExt* msg = (*it);
            if (m_msgTreeMap.find(msg->getQueueOffset()) == m_msgTreeMap.end())
            {
            	validMsgCnt++;
                m_queueOffsetMax = msg->getQueueOffset();
            }
            m_msgTreeMap[msg->getQueueOffset()] = msg;
        }
        m_msgCount += validMsgCnt;

        if (!m_msgTreeMap.empty() && !m_consuming)
        {
            dispathToConsume = true;
            m_consuming = true;
        }
    }
    catch (...)
    {
    	RMQ_ERROR("putMessage exception");
    }

    return dispathToConsume;
}

/**
* 获取当前队列的最大跨度
*/
long long ProcessQueue::getMaxSpan()
{
    try
    {
        kpr::ScopedRLock<kpr::RWMutex> lock(m_lockTreeMap);
        if (!m_msgTreeMap.empty())
        {
            std::map<long long, MessageExt*>::iterator it1 = m_msgTreeMap.begin();
            std::map<long long, MessageExt*>::iterator it2 = m_msgTreeMap.end();
            it2--;
            return it2->first - it1->first;
        }
    }
    catch (...)
    {
    	RMQ_ERROR("getMaxSpan exception");
    }

    return 0;
}

long long ProcessQueue::removeMessage(std::list<MessageExt*>& msgs)
{
    long long result = -1;
    unsigned long long now = KPRUtil::GetCurrentTimeMillis();
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
		m_lastConsumeTimestamp = now;

        if (!m_msgTreeMap.empty())
        {
            result = m_queueOffsetMax + 1;

			int removedCnt = 0;
            std::list<MessageExt*>::iterator it = msgs.begin();
            for (; it != msgs.end(); )
            {
                MessageExt* msg = (*it);
                if (m_msgTreeMap.find(msg->getQueueOffset()) != m_msgTreeMap.end())
	            {
	            	removedCnt++;
	            }
                m_msgTreeMap.erase(msg->getQueueOffset());

                //TODO delete message?
                it = msgs.erase(it);
                delete msg;
            }
			m_msgCount -= removedCnt;

			if (!m_msgTreeMap.empty())
	        {
	            std::map<long long, MessageExt*>::iterator it = m_msgTreeMap.begin();
	            result = it->first;
	        }
        }
    }
    catch (...)
    {
    	RMQ_ERROR("removeMessage exception");
    }

    return result;
}


void ProcessQueue::clear()
{
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
		m_msgTreeMap.clear();
		m_msgTreeMapTemp.clear();
		m_msgCount.set(0);
		m_queueOffsetMax = 0;
    }
    catch (...)
    {
    	RMQ_ERROR("clear exception");
    }

    return;
}


std::map<long long, MessageExt*> ProcessQueue::getMsgTreeMap()
{
    return m_msgTreeMap;
}

kpr::AtomicInteger ProcessQueue::getMsgCount()
{
    return m_msgCount;
}

bool ProcessQueue::isDropped()
{
    return m_dropped;
}

void ProcessQueue::setDropped(bool dropped)
{
    m_dropped = dropped;
}

bool ProcessQueue::isPullExpired()
{
    bool result = (KPRUtil::GetCurrentTimeMillis() - m_lastPullTimestamp) > s_PullMaxIdleTime;
    return result;
}

unsigned long long ProcessQueue::getLastPullTimestamp()
{
    return m_lastPullTimestamp;
}


void ProcessQueue::setLastPullTimestamp(unsigned long long lastPullTimestamp)
{
    m_lastPullTimestamp = lastPullTimestamp;
}


unsigned long long ProcessQueue::getLastConsumeTimestamp()
{
    return m_lastConsumeTimestamp;
}


void ProcessQueue::setLastConsumeTimestamp(unsigned long long lastConsumeTimestamp)
{
    m_lastConsumeTimestamp = lastConsumeTimestamp;
}



/**
* ========================================================================
* 以下部分为顺序消息专有操作
*/
kpr::Mutex& ProcessQueue::getLockConsume()
{
	return m_lockConsume;
}

void ProcessQueue::setLocked(bool locked)
{
    m_locked = locked;
}

bool ProcessQueue::isLocked()
{
    return m_locked;
}

long long ProcessQueue::getTryUnlockTimes()
{
   return m_tryUnlockTimes.get();
}

void ProcessQueue::incTryUnlockTimes()
{
   m_tryUnlockTimes++;
}


void ProcessQueue::rollback()
{
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
        m_msgTreeMap = m_msgTreeMapTemp;
        m_msgTreeMapTemp.clear();
    }
    catch (...)
    {
    	RMQ_ERROR("rollback exception");
    }
}

long long ProcessQueue::commit()
{
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
        if (!m_msgTreeMapTemp.empty())
        {
            std::map<long long, MessageExt*>::iterator it = m_msgTreeMapTemp.end();
            it--;
            long long offset = it->first;
            m_msgCount -= m_msgTreeMapTemp.size();
            m_msgTreeMapTemp.clear();
            return offset + 1;
        }
    }
    catch (...)
    {
    	RMQ_ERROR("commit exception");
    }

    return -1;
}

void ProcessQueue::makeMessageToCosumeAgain(const std::list<MessageExt*>& msgs)
{
    try
    {
        // 临时Table删除
        // 正常Table增加
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
        std::list<MessageExt*>::const_iterator it = msgs.begin();
        for (; it != msgs.end(); it++)
        {
            MessageExt* msg = (*it);
            m_msgTreeMapTemp.erase(msg->getQueueOffset());
            m_msgTreeMap[msg->getQueueOffset()] = msg;
        }
    }
    catch (...)
    {
    	RMQ_ERROR("makeMessageToCosumeAgain exception");
    }
}

/**
* 如果取不到消息，则将正在消费状态置为false
*
* @param batchSize
* @return
*/
std::list<MessageExt*> ProcessQueue::takeMessages(int batchSize)
{
    std::list<MessageExt*> result;
    unsigned long long now = KPRUtil::GetCurrentTimeMillis();
    try
    {
        kpr::ScopedWLock<kpr::RWMutex> lock(m_lockTreeMap);
        m_lastConsumeTimestamp = now;
        if (!m_msgTreeMap.empty())
        {
            for (int i = 0; i < batchSize; i++)
            {
                std::map<long long, MessageExt*>::iterator it = m_msgTreeMap.begin();
                if (it != m_msgTreeMap.end())
                {
                    result.push_back(it->second);
                    m_msgTreeMapTemp[it->first] = it->second;
                    m_msgTreeMap.erase(it);
                }
                else
                {
                    break;
                }
            }

            if (result.empty())
		    {
		        m_consuming = false;
		    }
        }
    }
    catch (...)
    {
    	RMQ_ERROR("takeMessags exception");
    }

    return result;
}

long long ProcessQueue::getLastLockTimestamp()
{
    return m_lastLockTimestamp;
}

void ProcessQueue::setLastLockTimestamp(long long lastLockTimestamp)
{
    m_lastLockTimestamp = lastLockTimestamp;
}


}
