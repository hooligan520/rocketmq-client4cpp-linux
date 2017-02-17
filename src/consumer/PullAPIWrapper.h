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

#ifndef __PULLAPIWRAPPER_H__
#define __PULLAPIWRAPPER_H__

#include <string>
#include <map>

#include "AtomicValue.h"
#include "PullResult.h"
#include "MessageQueue.h"
#include "CommunicationMode.h"
#include "Mutex.h"

namespace rmq
{
  class MQClientFactory;
  class PullCallback;
  class SubscriptionData;

  /**
  * 对Pull接口进行进一步的封装
  *
  */
  class PullAPIWrapper
  {
  public:
      PullAPIWrapper(MQClientFactory* pMQClientFactory, const std::string& consumerGroup);
      void updatePullFromWhichNode(MessageQueue& mq, long brokerId);

      /**
      * 对拉取结果进行处理，主要是消息反序列化
      *
      * @param mq
      * @param pullResult
      * @param subscriptionData
      * @param projectGroupPrefix
      *            虚拟环境projectGroupPrefix，不存在可设置为 null
      * @return
      */
      PullResult* processPullResult(MessageQueue& mq,
                                    PullResult& pullResult,
                                    SubscriptionData& subscriptionData);

      /**
      * 每个队列都应该有相应的变量来保存从哪个服务器拉
      */
      long recalculatePullFromWhichNode(MessageQueue& mq);

      PullResult* pullKernelImpl(MessageQueue& mq,
                                 const std::string& subExpression,
                                 long long subVersion,
                                 long long offset,
                                 int maxNums,
                                 int sysFlag,
                                 long long commitOffset,
                                 long long brokerSuspendMaxTimeMillis,
                                 int timeoutMillis,
                                 CommunicationMode communicationMode,
                                 PullCallback* pPullCallback);

  private:
      std::map<MessageQueue, kpr::AtomicInteger> m_pullFromWhichNodeTable;
      kpr::RWMutex m_pullFromWhichNodeTableLock;
      MQClientFactory* m_pMQClientFactory;
      std::string m_consumerGroup;
  };
}

#endif
