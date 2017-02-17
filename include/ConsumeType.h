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
#ifndef __RMQ_CONSUMETYPE_H__
#define __RMQ_CONSUMETYPE_H__

#include "RocketMQClient.h"

namespace rmq
{
	/**
	 * 消费类型
	 *
	 */
	enum ConsumeType
	{
		/**
		* 主动方式消费
		*/
		CONSUME_ACTIVELY,
		/**
		* 被动方式消费
		*/
		CONSUME_PASSIVELY,
	};

	enum ConsumeFromWhere
	{
		/**
		* 每次启动都从上次记录的位点开始消费，如果是第一次启动则从最大位点开始消费，建议在生产环境使用
		*/
		CONSUME_FROM_LAST_OFFSET,
		/**
		* 每次启动都从上次记录的位点开始消费，如果是第一次启动则从最小位点开始消费，建议测试时使用<br>
		* 线上环境此配置项可能需要审核，否则无效
		*/
		CONSUME_FROM_LAST_OFFSET_AND_FROM_MIN_WHEN_BOOT_FIRST,
		/**
		* 每次启动都从最小位点开始消费，建议测试时使用<br>
		* 线上环境此配置项可能需要审核，否则无效
		*/
		CONSUME_FROM_MIN_OFFSET,
		/**
		* 每次启动都从最大位点开始消费，建议测试时使用
		*/
		CONSUME_FROM_MAX_OFFSET,

		/**
	     * 一个新的订阅组第一次启动从队列的最前位置开始消费<br>
	     * 后续再启动接着上次消费的进度开始消费
	     */
	    CONSUME_FROM_FIRST_OFFSET,
	    /**
	     * 一个新的订阅组第一次启动从指定时间点开始消费<br>
	     * 后续再启动接着上次消费的进度开始消费<br>
	     * 时间点设置参见DefaultMQPushConsumer.consumeTimestamp参数
	     */
	    CONSUME_FROM_TIMESTAMP,
	};

	enum MessageModel
	{
		/**
		* 广播模型
		*/
		BROADCASTING,
		/**
		* 集群模型
		*/
		CLUSTERING,
		// /**
		// * 未知，如果是主动消费，很难确定应用的消息模型
		// */
		// UNKNOWNS,
	};

	const char* getConsumeTypeString(ConsumeType type);
	const char* getConsumeFromWhereString(ConsumeFromWhere type);
	const char* getMessageModelString(MessageModel type);
}

#endif
