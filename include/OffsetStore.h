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
#ifndef __RMQ_OFFSETSTORE_H__
#define __RMQ_OFFSETSTORE_H__

#include <set>
#include <map>

#include "RocketMQClient.h"

namespace rmq
{
	class MessageQueue;

	enum ReadOffsetType
	{
		// 只从Memory读取
		READ_FROM_MEMORY,
		// 只从存储层读取（本地或者远端）
		READ_FROM_STORE,
		// 先从内存读，内存不存在再从存储层读
		MEMORY_FIRST_THEN_STORE,
	};

	/**
	* Consumer Offset存储接口
	*
	*/
	class OffsetStore
	{
	public:
		virtual ~OffsetStore() {}

		/**
		* 加载Offset
		*/
		virtual void load()=0;

		/**
		* 更新消费进度，存储到内存
		*/
		virtual void updateOffset(const MessageQueue& mq, long long offset, bool increaseOnly)=0;

		/**
		* 从本地缓存读取消费进度
		*/
		virtual long long readOffset(const MessageQueue& mq, ReadOffsetType type)=0;

		/**
		* 持久化全部消费进度，可能持久化本地或者远端Broker
		*/
		virtual void persistAll(std::set<MessageQueue>& mqs)=0;
		virtual void persist(const MessageQueue& mq)=0;

		/**
		* 删除不必要的MessageQueue offset
		*/
		virtual void removeOffset(const MessageQueue& mq)=0;

		/**
		* 克隆偏移量表
		*/
		virtual std::map<MessageQueue, long long> cloneOffsetTable(const std::string& topic) = 0;
	};
}

#endif
