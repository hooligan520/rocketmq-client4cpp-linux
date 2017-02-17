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

#ifndef __MQCLIENTAPIIMPL_H__
#define __MQCLIENTAPIIMPL_H__

#include <string>
#include <map>
#include <list>
#include <set>

#include "ClientConfig.h"
#include "RemoteClientConfig.h"
#include "SubscriptionGroupConfig.h"
#include "TopicConfig.h"
#include "ConsumeStats.h"
#include "TopicStatsTable.h"
#include "KVTable.h"
#include "TopicRouteData.h"
#include "SendResult.h"
#include "PullResult.h"
#include "MessageExt.h"
#include "CommunicationMode.h"
#include "TopAddressing.h"
#include "HeartbeatData.h"
#include "LockBatchBody.h"

namespace rmq
{
class ClientConfig;
class TcpRemotingClient;
class QueryConsumerOffsetRequestHeader;
class UpdateConsumerOffsetRequestHeader;
class EndTransactionRequestHeader;
class SendMessageRequestHeader;
class PullMessageRequestHeader;
class QueryMessageRequestHeader;
class ProducerConnection;
class ConsumerConnection;
class ClusterInfo;
class TopicList;
class InvokeCallback;
class RemotingCommand;
class PullCallback;
class SendCallback;
class ClientRemotingProcessor;

/**
* 封装所有与服务器通信部分API
*
*/
class MQClientAPIImpl
{
  public:
      MQClientAPIImpl(ClientConfig& clientConfig,
	  				  const RemoteClientConfig& remoteClientConfig,
                      ClientRemotingProcessor* pClientRemotingProcessor);
      ~MQClientAPIImpl();

      void start();
      void shutdown();

      std::string getProjectGroupPrefix();
      std::vector<std::string> getNameServerAddressList();
      void updateNameServerAddressList(const std::string& addrs);
      std::string fetchNameServerAddr();

      void createSubscriptionGroup(const std::string& addr,
                                   SubscriptionGroupConfig config,
                                   int timeoutMillis);

      void createTopic(const std::string& addr,
                       const std::string& defaultTopic,
                       TopicConfig topicConfig,
                       int timeoutMillis);

      SendResult sendMessage(const std::string& addr,
                             const std::string& brokerName,
                             Message& msg,
                             SendMessageRequestHeader* pRequestHeader,
                             int timeoutMillis,
                             CommunicationMode communicationMode,
                             SendCallback* pSendCallback);

      PullResult* pullMessage(const std::string& addr,
                              PullMessageRequestHeader* pRequestHeader,
                              int timeoutMillis,
                              CommunicationMode communicationMode,
                              PullCallback* pPullCallback);

      MessageExt* viewMessage(const std::string& addr, long long phyoffset, int timeoutMillis);

      /**
      * 根据时间查询Offset
      */
      long long searchOffset(const std::string& addr,
                             const std::string& topic,
                             int queueId,
                             long long timestamp,
                             int timeoutMillis);

      /**
      * 获取队列的最大Offset
      */
      long long getMaxOffset(const std::string& addr,
                             const std::string& topic,
                             int queueId,
                             int timeoutMillis);

      /**
      * 获取某个组的Consumer Id列表
      */
      std::list<std::string> getConsumerIdListByGroup(const std::string& addr,
              const std::string& consumerGroup,
              int timeoutMillis);

      /**
      * 获取队列的最小Offset
      */
      long long getMinOffset(const std::string& addr,
                             const std::string& topic,
                             int queueId,
                             int timeoutMillis);
      /**
      * 获取队列的最早时间
      */
      long long getEarliestMsgStoretime(const std::string& addr,
                                        const std::string& topic,
                                        int queueId,
                                        int timeoutMillis);

      /**
      * 查询Consumer消费进度
      */
      long long queryConsumerOffset(const std::string& addr,
                                    QueryConsumerOffsetRequestHeader* pRequestHeader,
                                    int timeoutMillis);

      /**
      * 更新Consumer消费进度
      */
      void updateConsumerOffset(const std::string& addr,
                                UpdateConsumerOffsetRequestHeader* pRequestHeader,
                                int timeoutMillis);

      /**
      * 更新Consumer消费进度
      *
      * @throws InterruptedException
      * @throws RemotingSendRequestException
      * @throws RemotingTimeoutException
      * @throws RemotingTooMuchRequestException
      *
      * @throws RemotingConnectException
      */
      void updateConsumerOffsetOneway(const std::string& addr,
                                      UpdateConsumerOffsetRequestHeader* pRequestHeader,
                                      int timeoutMillis);

      /**
      * 发送心跳
      */
      void sendHearbeat(const std::string& addr, HeartbeatData* pHeartbeatData, int timeoutMillis);

      void unregisterClient(const std::string& addr,
                            const std::string& clientID,
                            const std::string& producerGroup,
                            const std::string& consumerGroup,
                            int timeoutMillis);

      /**
      * 提交或者回滚事务
      */
      void endTransactionOneway(const std::string& addr,
                                EndTransactionRequestHeader* pRequestHeader,
                                const std::string& remark,
                                int timeoutMillis);

      /**
      * 查询消息
      */
      void queryMessage(const std::string& addr,
                        QueryMessageRequestHeader* pRequestHeader,
                        int timeoutMillis,
                        InvokeCallback* pInvokeCallback);

      bool registerClient(const std::string& addr,
                          HeartbeatData& heartbeat,
                          int timeoutMillis);

      /**
      * 失败的消息发回Broker
      */
      void consumerSendMessageBack(const std::string& addr,
      							   MessageExt& msg,
                                   const std::string& consumerGroup,
                                   int delayLevel,
                                   int timeoutMillis);

      std::set<MessageQueue> lockBatchMQ(const std::string& addr,
                                         LockBatchRequestBody* pRequestBody,
                                         int timeoutMillis);

      void unlockBatchMQ(const std::string& addr,
                         UnlockBatchRequestBody* pRequestBody,
                         int timeoutMillis,
                         bool oneway);

      TopicStatsTable getTopicStatsInfo(const std::string& addr,
                                        const std::string& topic,
                                        int timeoutMillis);

      ConsumeStats getConsumeStats(const std::string& addr,
                                   const std::string& consumerGroup,
                                   int timeoutMillis);

      /**
      * 根据ProducerGroup获取Producer连接列表
      */
      ProducerConnection* getProducerConnectionList(const std::string& addr,
              const std::string& producerGroup,
              int timeoutMillis);

      /**
      * 根据ConsumerGroup获取Consumer连接列表以及订阅关系
      */
      ConsumerConnection* getConsumerConnectionList(const std::string& addr,
              const std::string& consumerGroup,
              int timeoutMillis);

      KVTable getBrokerRuntimeInfo(const std::string& addr,  int timeoutMillis);

      /**
      * 更新Broker的配置文件
      *
      * @param addr
      * @param properties
      * @param timeoutMillis
      * @throws RemotingConnectException
      * @throws RemotingSendRequestException
      * @throws RemotingTimeoutException
      * @throws InterruptedException
      * @throws MQBrokerException
      * @throws UnsupportedEncodingException
      */
      void updateBrokerConfig(const std::string& addr,
                              const std::map<std::string, std::string>& properties,
                              int timeoutMillis);

      /**
      * Name Server: 从Name Server获取集群信息
      */
      ClusterInfo* getBrokerClusterInfo(int timeoutMillis);

      /**
      * Name Server: 从Name Server获取 Default Topic 路由信息
      */
      TopicRouteData* getDefaultTopicRouteInfoFromNameServer(const std::string& topic, int timeoutMillis);

      /**
      * Name Server: 从Name Server获取Topic路由信息
      */
      TopicRouteData* getTopicRouteInfoFromNameServer(const std::string& topic, int timeoutMillis);

      /**
      * Name Server: 从Name Server获取所有Topic列表
      */
      TopicList* getTopicListFromNameServer(int timeoutMillis);

      /**
      * Name Server: Broker下线前，清除Broker对应的权限
      */
      int wipeWritePermOfBroker(const std::string& namesrvAddr,
                                const std::string& brokerName,
                                int timeoutMillis);

      void deleteTopicInBroker(const std::string& addr, const std::string& topic, int timeoutMillis);
      void deleteTopicInNameServer(const std::string& addr, const std::string& topic, int timeoutMillis);
      void deleteSubscriptionGroup(const std::string& addr,
                                   const std::string& groupName,
                                   int timeoutMillis);

      /**
      * Name Server: 从Namesrv获取KV配置
      */
      std::string getKVConfigValue(const std::string& projectNamespace,
                                   const std::string& key,
                                   int timeoutMillis);

      /**
      * Name Server: 添加KV配置
      */
      void putKVConfigValue(const std::string& projectNamespace,
                            const std::string& key,
                            const std::string& value,
                            int timeoutMillis);

      /**
      * Name Server: 添加KV配置
      */
      void deleteKVConfigValue(const std::string& projectNamespace, const std::string& key, int timeoutMillis);

      /**
      * Name Server: 通过 server ip 获取 project 信息
      */
      std::string getProjectGroupByIp(const std::string& ip,  int timeoutMillis);

      /**
      * Name Server: 通过 value 获取所有的 key 信息
      */
      std::string getKVConfigByValue(const std::string& projectNamespace,
                                     const std::string& projectGroup,
                                     int timeoutMillis);

      /**
      * Name Server: 获取指定Namespace下的所有KV
      */
      KVTable getKVListByNamespace(const std::string& projectNamespace,  int timeoutMillis);

      /**
      * Name Server: 删除 value 对应的所有 key
      */
      void deleteKVConfigByValue(const std::string& projectNamespace,
                                 const std::string& projectGroup,
                                 int timeoutMillis);

      TcpRemotingClient* getRemotingClient();

      SendResult* processSendResponse(const std::string& brokerName,
                                      const std::string& topic,
                                      RemotingCommand* pResponse);

      PullResult* processPullResponse(RemotingCommand* pResponse);

  private:
      SendResult* sendMessageSync(const std::string& addr,
                                  const std::string& brokerName,
                                  Message& msg,
                                  int timeoutMillis,
                                  RemotingCommand* request);

      void sendMessageAsync(const std::string& addr,
                            const std::string& brokerName,
                            Message& msg,
                            int timeoutMillis,
                            RemotingCommand* request,
                            SendCallback* pSendCallback);

      void pullMessageAsync(const std::string& addr,
                            RemotingCommand* pRequest,
                            int timeoutMillis,
                            PullCallback* pPullCallback);

      PullResult* pullMessageSync(const std::string& addr,
                                  RemotingCommand* pRequest,
                                  int timeoutMillis);

  private:
      TcpRemotingClient* m_pRemotingClient;
      TopAddressing m_topAddressing;
      ClientRemotingProcessor* m_pClientRemotingProcessor;
      std::string m_nameSrvAddr;
      std::string m_projectGroupPrefix;// 虚拟运行环境相关的project group ,c++是否需要？
  };
}

#endif
