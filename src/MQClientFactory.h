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

#ifndef __MQCLIENTFACTORY_H__
#define __MQCLIENTFACTORY_H__

#include <set>
#include <string>
#include <list>

#include "SocketUtil.h"
#include "TopicRouteData.h"
#include "FindBrokerResult.h"
#include "ClientConfig.h"
#include "Mutex.h"
#include "ServiceState.h"
#include "TimerTaskManager.h"

namespace rmq
{
    class ClientConfig;
    class MessageQueue;
    class MQAdminExtInner;
    class MQClientAPIImpl;
    class MQAdminImpl;
    class PullMessageService;
    class HeartbeatData;
    class RemoteClientConfig;
    class ClientRemotingProcessor;
    class RebalanceService;
    class DefaultMQProducer;
    class TopicPublishInfo;
    class MQProducerInner;
    class MQConsumerInner;
    class DefaultMQProducerImpl;

    /**
    * 客户端Factory类，用来管理Producer与Consumer
    *
    */

    class MQClientFactory
    {
    public:
        MQClientFactory(ClientConfig& clientConfig, int factoryIndex, const std::string& clientId);
        ~MQClientFactory();

        void start();
        void shutdown();
        void sendHeartbeatToAllBrokerWithLock();
        void updateTopicRouteInfoFromNameServer();
        bool updateTopicRouteInfoFromNameServer(const std::string& topic);

        /**
        * 调用Name Server接口，根据Topic获取路由信息
        */
        bool updateTopicRouteInfoFromNameServer(const std::string& topic, bool isDefault,
                                                DefaultMQProducer* pDefaultMQProducer);

        static TopicPublishInfo* topicRouteData2TopicPublishInfo(const std::string& topic,
                TopicRouteData& route);

        static std::set<MessageQueue>* topicRouteData2TopicSubscribeInfo(const std::string& topic,
                TopicRouteData& route);

        bool registerConsumer(const std::string& group, MQConsumerInner* pConsumer);
        void unregisterConsumer(const std::string& group);

        bool registerProducer(const std::string& group, DefaultMQProducerImpl* pProducer);
        void unregisterProducer(const std::string& group);

        bool registerAdminExt(const std::string& group, MQAdminExtInner* pAdmin);
        void unregisterAdminExt(const std::string& group);

        void rebalanceImmediately();
        void doRebalance();

        MQProducerInner* selectProducer(const std::string& group);
        MQConsumerInner* selectConsumer(const std::string& group);

        /**
        * 管理类的接口查询Broker地址，Master优先
        *
        * @param brokerName
        * @return
        */
        FindBrokerResult findBrokerAddressInAdmin(const std::string& brokerName);

        /**
        * 发布消息过程中，寻找Broker地址，一定是找Master
        */
        std::string findBrokerAddressInPublish(const std::string& brokerName);

        /**
        * 订阅消息过程中，寻找Broker地址，取Master还是Slave由参数决定
        */
        FindBrokerResult findBrokerAddressInSubscribe(//
            const std::string& brokerName,//
            long brokerId,//
            bool onlyThisBroker);

        std::list<std::string> findConsumerIdList(const std::string& topic, const std::string& group);
        std::string findBrokerAddrByTopic(const std::string& topic);
        TopicRouteData getAnExistTopicRouteData(const std::string& topic);
        MQClientAPIImpl* getMQClientAPIImpl();
        MQAdminImpl* getMQAdminImpl();
        std::string getClientId();
        long long getBootTimestamp();
        PullMessageService* getPullMessageService();
        DefaultMQProducer* getDefaultMQProducer();

    private:
        void sendHeartbeatToAllBroker();
        //HeartbeatData* prepareHeartbeatData();
        void prepareHeartbeatData(HeartbeatData& heartbeatData);

        void makesureInstanceNameIsOnly(const std::string& instanceName);
        void startScheduledTask();

        /**
        * 清理下线的broker
        */
        void cleanOfflineBroker();
        bool isBrokerAddrExistInTopicRouteTable(const std::string& addr);
        void recordSnapshotPeriodically();
        void logStatsPeriodically();
        void persistAllConsumerOffset();
        bool topicRouteDataIsChange(TopicRouteData& olddata, TopicRouteData& nowdata);
        bool isNeedUpdateTopicRouteInfo(const std::string& topic);
        void unregisterClientWithLock(const std::string& producerGroup, const std::string& consumerGroup);
        void unregisterClient(const std::string& producerGroup, const std::string& consumerGroup);

        typedef void (MQClientFactory::*pScheduledFunc)();

        class ScheduledTask : public kpr::TimerTask
        {
        public:
            ScheduledTask(MQClientFactory* pMQClientFactory, pScheduledFunc pScheduled)
                : m_pMQClientFactory(pMQClientFactory), m_pScheduled(pScheduled)
            {
            }

            virtual void DoTask()
            {
                (m_pMQClientFactory->*m_pScheduled)();
            }

        private:
            MQClientFactory* m_pMQClientFactory;
            pScheduledFunc m_pScheduled;
        };
		typedef kpr::RefHandleT<ScheduledTask> ScheduledTaskPtr;

        //定时任务
        void fetchNameServerAddr();
        void updateTopicRouteInfoFromNameServerTask();
        void cleanBroker();
        void persistAllConsumerOffsetTask();
        void recordSnapshotPeriodicallyTask();
        void logStatsPeriodicallyTask();

    private:
        static long LockTimeoutMillis;
        ClientConfig m_clientConfig;
        int m_factoryIndex;
        std::string m_clientId;
        long long m_bootTimestamp;

        // Producer对象
        //group --> MQProducerInner
        std::map<std::string, MQProducerInner*> m_producerTable;
        kpr::RWMutex m_producerTableLock;

        // Consumer对象
        //group --> MQConsumerInner
        std::map<std::string, MQConsumerInner*> m_consumerTable;
        kpr::RWMutex m_consumerTableLock;

        // AdminExt对象
        // group --> MQAdminExtInner
        std::map<std::string, MQAdminExtInner*> m_adminExtTable;
        kpr::RWMutex m_adminExtTableLock;

        // 远程客户端配置
        RemoteClientConfig* m_pRemoteClientConfig;

        // RPC调用的封装类
        MQClientAPIImpl* m_pMQClientAPIImpl;
        MQAdminImpl* m_pMQAdminImpl;

        // 存储从Name Server拿到的Topic路由信息
        /// Topic---> TopicRouteData
        std::map<std::string, TopicRouteData> m_topicRouteTable;
        kpr::RWMutex m_topicRouteTableLock;

        kpr::Mutex m_mutex;
        // 调用Name Server获取Topic路由信息时，加锁
        kpr::Mutex m_lockNamesrv;

        // 心跳与注销动作加锁
        kpr::Mutex m_lockHeartbeat;

        // 存储Broker Name 与Broker Address的对应关系
        //
        //-----brokerName
        //     ------brokerid  addr
        //     ------brokerid  addr
        std::map<std::string, std::map<int, std::string> > m_brokerAddrTable;
        kpr::RWMutex m_brokerAddrTableLock;

        // 定时线程
        kpr::TimerTaskManager m_timerTaskManager;
        ScheduledTaskPtr m_pFetchNameServerAddrTask;
        ScheduledTaskPtr m_pUpdateTopicRouteInfoFromNameServerTask;
        ScheduledTaskPtr m_pCleanBrokerTask;
        ScheduledTaskPtr m_pPersistAllConsumerOffsetTask;
        ScheduledTaskPtr m_pRecordSnapshotPeriodicallyTask;
        ScheduledTaskPtr m_pLogStatsPeriodicallyTask;

        int m_scheduledTaskIds[6];

        ClientRemotingProcessor* m_pClientRemotingProcessor;// 处理服务器主动发来的请求
        PullMessageService* m_pPullMessageService;// 拉消息服务
        RebalanceService* m_pRebalanceService;// Rebalance服务
        DefaultMQProducer* m_pDefaultMQProducer;// 内置Producer对象
        ServiceState m_serviceState;

        // 监听一个UDP端口，用来防止同一个Factory启动多份（有可能分布在多个JVM中）？？C++是否需要
        //SOCKET m_datagramSocket;
    };
}

#endif
