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

#ifndef __REMOTECLIENTCONFIG_H__
#define __REMOTECLIENTCONFIG_H__

#include <unistd.h>
#include <sys/sysinfo.h>

namespace rmq
{
    /**
     * remote客户端配置类
     *
     */
    class RemoteClientConfig
    {
    public:
        RemoteClientConfig()
        {
            clientWorkerThreads = 4;
            clientCallbackExecutorThreads = get_nprocs();//可用cpu个数
            clientSelectorThreads = 1;
            clientOnewaySemaphoreValue = 2048;
            clientAsyncSemaphoreValue = 2048;
            connectTimeoutMillis = 3000;
            channelNotActiveInterval = 1000 * 60;// channel超过1分钟不被访问 就关闭
            clientChannelMaxIdleTimeSeconds = 120;
			clientSocketSndBufSize = 65535;
			clientSocketRcvBufSize = 65535;

			nsL5ModId = 0;
			nsL5CmdId = 0;
        }

        // 处理Server Response/Request
        int clientWorkerThreads;
        int clientCallbackExecutorThreads;//cpu个数
        int clientSelectorThreads;
        int clientOnewaySemaphoreValue;
        int clientAsyncSemaphoreValue;
        int connectTimeoutMillis;

        int channelNotActiveInterval;// channel超过1分钟不被访问 就关闭
        int clientChannelMaxIdleTimeSeconds;
		int clientSocketSndBufSize;
		int clientSocketRcvBufSize;

		// namesrv的l5配置
		int nsL5ModId;
		int nsL5CmdId;
    };
}

#endif
