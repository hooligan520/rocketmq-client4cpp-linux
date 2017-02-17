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

#include "ConsumeMessageOrderlyService.h"

#include <list>
#include <string>

#include "ScopedLock.h"
#include "DefaultMQPushConsumerImpl.h"
#include "MessageListener.h"
#include "MessageQueue.h"
#include "RebalanceImpl.h"
#include "DefaultMQPushConsumer.h"
#include "OffsetStore.h"
#include "KPRUtil.h"
#include "MixAll.h"

namespace rmq
{

class LockMq : public kpr::TimerHandler
{
public:
    LockMq(ConsumeMessageOrderlyService* pService)
        : m_pService(pService)
    {

    }

    void OnTimeOut(unsigned int timerID)
    {
        m_pService->lockMQPeriodically();

        // 持续运行不能删除
        //delete this;
    }

private:
    ConsumeMessageOrderlyService* m_pService;
};

long ConsumeMessageOrderlyService::s_MaxTimeConsumeContinuously = 60000;

ConsumeMessageOrderlyService::ConsumeMessageOrderlyService(DefaultMQPushConsumerImpl* pDefaultMQPushConsumerImpl,
        MessageListenerOrderly* pMessageListener)
{
	m_stoped = false;
    m_pDefaultMQPushConsumerImpl = pDefaultMQPushConsumerImpl;
    m_pMessageListener = pMessageListener;
    m_pDefaultMQPushConsumer = m_pDefaultMQPushConsumerImpl->getDefaultMQPushConsumer();
    m_consumerGroup = m_pDefaultMQPushConsumer->getConsumerGroup();
    m_pConsumeExecutor = new kpr::ThreadPool("ConsumeMessageThreadPool", 1,
    m_pDefaultMQPushConsumer->getConsumeThreadMin(), m_pDefaultMQPushConsumer->getConsumeThreadMax());
    m_scheduledExecutorService = new kpr::TimerThread("ConsumeMessageConcurrentlyService", 10);
}

ConsumeMessageOrderlyService::~ConsumeMessageOrderlyService()
{
}


void ConsumeMessageOrderlyService::start()
{
    m_scheduledExecutorService->Start();

    LockMq* lm = new LockMq(this);
    m_scheduledExecutorService->RegisterTimer(0, ProcessQueue::s_RebalanceLockInterval, lm, true);
}

void ConsumeMessageOrderlyService::shutdown()
{
    m_stoped = true;
    m_pConsumeExecutor->Destroy();
    m_scheduledExecutorService->Stop();
    m_scheduledExecutorService->Join();
    unlockAllMQ();
}

void ConsumeMessageOrderlyService::unlockAllMQ()
{
    m_pDefaultMQPushConsumerImpl->getRebalanceImpl()->unlockAll(false);
}

void ConsumeMessageOrderlyService::lockMQPeriodically()
{
    if (!m_stoped)
    {
        m_pDefaultMQPushConsumerImpl->getRebalanceImpl()->lockAll();
    }
}

bool ConsumeMessageOrderlyService::lockOneMQ(MessageQueue& mq)
{
    if (!m_stoped)
    {
        return m_pDefaultMQPushConsumerImpl->getRebalanceImpl()->lock(mq);
    }

    return false;
}

class TryLockLaterAndReconsume : public kpr::TimerHandler
{
public:
    TryLockLaterAndReconsume(ProcessQueue* pProcessQueue,
                             MessageQueue& messageQueue,
                             ConsumeMessageOrderlyService* pService)
        : m_pProcessQueue(pProcessQueue),
          m_messageQueue(messageQueue),
          m_pService(pService)
    {

    }

    void OnTimeOut(unsigned int timerID)
    {
    	try
    	{
	        bool lockOK = m_pService->lockOneMQ(m_messageQueue);
	        if (lockOK)
	        {
	            m_pService->submitConsumeRequestLater(m_pProcessQueue, m_messageQueue, 10);
	        }
	        else
	        {
	            m_pService->submitConsumeRequestLater(m_pProcessQueue, m_messageQueue, 3000);
	        }
        }
        catch(...)
        {
        	RMQ_ERROR("TryLockLaterAndReconsume OnTimeOut exception");
        }

        delete this;
    }

private:
    ProcessQueue* m_pProcessQueue;
    MessageQueue m_messageQueue;
    ConsumeMessageOrderlyService* m_pService;
};

void ConsumeMessageOrderlyService::tryLockLaterAndReconsume(MessageQueue& messageQueue,
        ProcessQueue* pProcessQueue,
        long long delayMills)
{
    TryLockLaterAndReconsume* consume = new TryLockLaterAndReconsume(pProcessQueue, messageQueue, this);
    m_scheduledExecutorService->RegisterTimer(0, int(delayMills), consume, false);
}

ConsumerStat& ConsumeMessageOrderlyService::getConsumerStat()
{
    return m_pDefaultMQPushConsumerImpl->getConsumerStatManager()->getConsumertat();
}

class SubmitConsumeRequestLaterOrderly : public kpr::TimerHandler
{
public:
    SubmitConsumeRequestLaterOrderly(ProcessQueue* pProcessQueue,
                                     const MessageQueue& messageQueue,
                                     ConsumeMessageOrderlyService* pService)
        : m_pProcessQueue(pProcessQueue),
          m_messageQueue(messageQueue),
          m_pService(pService)
    {

    }

    void OnTimeOut(unsigned int timerID)
    {
    	try
    	{
        	std::list<MessageExt*> msgs;
       		m_pService->submitConsumeRequest(msgs, m_pProcessQueue, m_messageQueue, true);
        }
        catch(...)
        {
        	RMQ_ERROR("SubmitConsumeRequestLaterOrderly OnTimeOut exception");
        }

        delete this;
    }

private:
    ProcessQueue* m_pProcessQueue;
    MessageQueue m_messageQueue;
    ConsumeMessageOrderlyService* m_pService;
};

void ConsumeMessageOrderlyService::submitConsumeRequestLater(ProcessQueue* pProcessQueue,
        const MessageQueue& messageQueue,
        long long suspendTimeMillis)
{
    long timeMillis = long(suspendTimeMillis);
    if (timeMillis < 10)
    {
        timeMillis = 10;
    }
    else if (timeMillis > 30000)
    {
        timeMillis = 30000;
    }

    SubmitConsumeRequestLaterOrderly* sc = new SubmitConsumeRequestLaterOrderly(pProcessQueue, messageQueue, this);
    m_scheduledExecutorService->RegisterTimer(0, timeMillis, sc, false);
}

void ConsumeMessageOrderlyService::submitConsumeRequest(std::list<MessageExt*>& msgs,
        ProcessQueue* pProcessQueue,
        const MessageQueue& messageQueue,
        bool dispathToConsume)
{
    if (dispathToConsume)
    {
        kpr::ThreadPoolWorkPtr consumeRequest = new ConsumeOrderlyRequest(pProcessQueue, messageQueue, this);
        m_pConsumeExecutor->AddWork(consumeRequest);
    }
}

void ConsumeMessageOrderlyService::updateCorePoolSize(int corePoolSize)
{
}


std::string& ConsumeMessageOrderlyService::getConsumerGroup()
{
    return m_consumerGroup;
}

MessageListenerOrderly* ConsumeMessageOrderlyService::getMessageListener()
{
    return m_pMessageListener;
}

DefaultMQPushConsumerImpl* ConsumeMessageOrderlyService::getDefaultMQPushConsumerImpl()
{
    return m_pDefaultMQPushConsumerImpl;
}

bool ConsumeMessageOrderlyService::processConsumeResult(std::list<MessageExt*>& msgs,
        ConsumeOrderlyStatus status,
        ConsumeOrderlyContext& context,
        ConsumeOrderlyRequest& consumeRequest)
{
    bool continueConsume = true;
    long long commitOffset = -1L;
    int msgsSize = msgs.size();

    // 非事务方式，自动提交
    if (context.autoCommit)
    {
        switch (status)
        {
            case COMMIT:
            case ROLLBACK:
                //TODO log.warn("the message queue consume result is illegal, we think you want to ack these message {}",
                //  consumeRequest.getMessageQueue());
            case SUCCESS:
                commitOffset = consumeRequest.getProcessQueue()->commit();
                // 统计信息
                getConsumerStat().consumeMsgOKTotal.fetchAndAdd(msgsSize);
                break;
            case SUSPEND_CURRENT_QUEUE_A_MOMENT:
                consumeRequest.getProcessQueue()->makeMessageToCosumeAgain(msgs);
                submitConsumeRequestLater(consumeRequest.getProcessQueue(),
                                          consumeRequest.getMessageQueue(),
                                          context.suspendCurrentQueueTimeMillis);
                continueConsume = false;

                // 统计信息
                getConsumerStat().consumeMsgFailedTotal.fetchAndAdd(msgsSize);
                break;
            default:
                break;
        }
    }
    // 事务方式，由用户来控制提交回滚
    else
    {
        switch (status)
        {
            case SUCCESS:
                // 统计信息
                getConsumerStat().consumeMsgOKTotal.fetchAndAdd(msgsSize);
                break;
            case COMMIT:
                commitOffset = consumeRequest.getProcessQueue()->commit();
                // 统计信息
                getConsumerStat().consumeMsgOKTotal.fetchAndAdd(msgsSize);
                break;
            case ROLLBACK:
                // 如果Rollback后，最好suspend一会儿再消费，防止应用无限Rollback下去
                consumeRequest.getProcessQueue()->rollback();
                submitConsumeRequestLater(consumeRequest.getProcessQueue(),
                                          consumeRequest.getMessageQueue(),
                                          context.suspendCurrentQueueTimeMillis);
                continueConsume = false;
                // 统计信息
                getConsumerStat().consumeMsgFailedTotal.fetchAndAdd(msgsSize);
                break;
            case SUSPEND_CURRENT_QUEUE_A_MOMENT:
                consumeRequest.getProcessQueue()->makeMessageToCosumeAgain(msgs);
                submitConsumeRequestLater(consumeRequest.getProcessQueue(),
                                          consumeRequest.getMessageQueue(),
                                          context.suspendCurrentQueueTimeMillis);
                continueConsume = false;
                // 统计信息
                getConsumerStat().consumeMsgFailedTotal.fetchAndAdd(msgsSize);
                break;
            default:
                break;
        }
    }

    if (commitOffset >= 0)
    {
        m_pDefaultMQPushConsumerImpl->getOffsetStore()->updateOffset(consumeRequest.getMessageQueue(),
                commitOffset, false);
    }

    return continueConsume;
}

MessageQueueLock& ConsumeMessageOrderlyService::getMessageQueueLock()
{
    return m_messageQueueLock;
}

DefaultMQPushConsumer* ConsumeMessageOrderlyService::getDefaultMQPushConsumer()
{
    return m_pDefaultMQPushConsumer;
}

ConsumeOrderlyRequest::ConsumeOrderlyRequest(ProcessQueue* pProcessQueue,
        const MessageQueue& messageQueue,
        ConsumeMessageOrderlyService* pService)
{
	m_pProcessQueue = pProcessQueue;
	m_messageQueue = messageQueue;
	m_pService = pService;
}

ConsumeOrderlyRequest::~ConsumeOrderlyRequest()
{
}

void ConsumeOrderlyRequest::Do()
{
	if (m_pProcessQueue->isDropped())
	{
        RMQ_WARN("run, the message queue not be able to consume, because it's dropped, MQ: %s",
            m_messageQueue.toString().c_str());
        return;
    }

	try
	{
	    // 保证在当前Consumer内，同一队列串行消费
	    kpr::Mutex* objLock = m_pService->getMessageQueueLock().fetchLockObject(m_messageQueue);
	    {
	        kpr::ScopedLock<kpr::Mutex> lock(*objLock);

	        // 保证在Consumer集群，同一队列串行消费
	        MessageModel messageModel = m_pService->getDefaultMQPushConsumerImpl()->messageModel();
	        if (BROADCASTING == messageModel
	        	|| (m_pProcessQueue->isLocked() || !m_pProcessQueue->isLockExpired()))
	        {
	            long long beginTime = KPRUtil::GetCurrentTimeMillis();
	            for (bool continueConsume = true; continueConsume;)
	            {
	                if (m_pProcessQueue->isDropped())
	                {
	                    RMQ_INFO("the message queue not be able to consume, because it's droped, MQ: %s",
	                             m_messageQueue.toString().c_str());
	                    break;
	                }

	                if (CLUSTERING == messageModel
	                 	&& !m_pProcessQueue->isLocked())
	                {
	                    RMQ_WARN("the message queue not locked, so consume later, MQ: %s", m_messageQueue.toString().c_str());

	                    //TODO 暂时打开
	                    m_pService->tryLockLaterAndReconsume(m_messageQueue, m_pProcessQueue, 10);
	                    break;
	                }

	                if (CLUSTERING == messageModel
	                 	&& m_pProcessQueue->isLockExpired())
	                {
	                    RMQ_WARN("the message queue lock expired, so consume later, MQ: %s", m_messageQueue.toString().c_str());

	                    //TODO 暂时打开
	                    m_pService->tryLockLaterAndReconsume(m_messageQueue, m_pProcessQueue, 10);
	                    break;
	                }

	                // 在线程数小于队列数情况下，防止个别队列被饿死
	                long interval = long(KPRUtil::GetCurrentTimeMillis() - beginTime);
	                if (interval > ConsumeMessageOrderlyService::s_MaxTimeConsumeContinuously)
	                {
	                    // 过10ms后再消费
	                    m_pService->submitConsumeRequestLater(m_pProcessQueue, m_messageQueue, 10);
	                    break;
	                }

	                int consumeBatchSize =
	                    m_pService->getDefaultMQPushConsumer()->getConsumeMessageBatchMaxSize();

	                std::list<MessageExt*> msgs = m_pProcessQueue->takeMessages(consumeBatchSize);
	                if (!msgs.empty())
	                {
	                    ConsumeOrderlyContext context(m_messageQueue);

	                    ConsumeOrderlyStatus status = SUSPEND_CURRENT_QUEUE_A_MOMENT;

	                    // 执行Hook
	                    ConsumeMessageContext consumeMessageContext;
	                    if (m_pService->getDefaultMQPushConsumerImpl()->hasHook())
	                    {
	                        consumeMessageContext.consumerGroup = m_pService->getConsumerGroup();
	                        consumeMessageContext.mq = m_messageQueue;
	                        consumeMessageContext.msgList = msgs;
	                        consumeMessageContext.success = false;
	                        m_pService->getDefaultMQPushConsumerImpl()->executeHookBefore(consumeMessageContext);
	                    }

	                    long long beginTimestamp = KPRUtil::GetCurrentTimeMillis();
	                    try
	                    {
	                    	kpr::ScopedLock<kpr::Mutex> lock(m_pProcessQueue->getLockConsume());
	                    	if (m_pProcessQueue->isDropped())
							{
						        RMQ_WARN("consumeMessage, the message queue not be able to consume, because it's dropped, MQ: %s",
						            m_messageQueue.toString().c_str());
						        break;
						    }

	                        status = m_pService->getMessageListener()->consumeMessage(msgs, context);
	                    }
	                    catch (...)
	                    {
	                        RMQ_WARN("consumeMessage exception, Group: {%s}, Msgs: {%u}, MQ: %s",//
	                                 m_pService->getConsumerGroup().c_str(),
	                                 (unsigned)msgs.size(),
	                                 m_messageQueue.toString().c_str());
	                    }

	                    long long consumeRT = KPRUtil::GetCurrentTimeMillis() - beginTimestamp;

	                    // 用户抛出异常或者返回null，都挂起队列
	                    if (SUSPEND_CURRENT_QUEUE_A_MOMENT == status
	                    	|| ROLLBACK == status)
	                    {
	                    	RMQ_WARN("consumeMessage Orderly return not OK, Group: {%s} Msgs: {%u} MQ: %s",//
	                                    m_pService->getConsumerGroup().c_str(),
	                                    (unsigned)msgs.size(),
	                                     m_messageQueue.toString().c_str());
	                    	//status = ConsumeOrderlyStatus.SUSPEND_CURRENT_QUEUE_A_MOMENT;
	                    }

	                    // 执行Hook
	                    if (m_pService->getDefaultMQPushConsumerImpl()->hasHook())
	                    {
	                        consumeMessageContext.success = (SUCCESS == status
	                                                         || COMMIT == status);
	                        m_pService->getDefaultMQPushConsumerImpl()->executeHookAfter(consumeMessageContext);
	                    }

	                    // 记录统计信息
	                    m_pService->getConsumerStat().consumeMsgRTTotal.fetchAndAdd(consumeRT);
	                    MixAll::compareAndIncreaseOnly(m_pService->getConsumerStat()
	                                                   .consumeMsgRTMax, consumeRT);

	                    continueConsume = m_pService->processConsumeResult(msgs, status, context, *this);
	                }
	                else
	                {
	                    continueConsume = false;
	                }
	            }
	        }
	        // 没有拿到当前队列的锁，稍后再消费
	        else
	        {
	        	if (m_pProcessQueue->isDropped())
				{
			        RMQ_WARN("consumeMessage, the message queue not be able to consume, because it's dropped, MQ: %s",
			            m_messageQueue.toString().c_str());
			        return;
			    }

	            m_pService->tryLockLaterAndReconsume(m_messageQueue, m_pProcessQueue, 100);
	        }
	    }
	}
	catch(...)
	{
		RMQ_WARN("ConsumeOrderlyRequest exception");
	}

    return;
}

}
