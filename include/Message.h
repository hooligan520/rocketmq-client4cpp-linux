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

#ifndef __RMQ_MESSAGE_H__
#define __RMQ_MESSAGE_H__

#include <map>
#include <string>
#include <list>
#include "RocketMQClient.h"

namespace rmq
{
	/**
	* 消息，Producer与Consumer使用
	*
	*/
	class Message
	{
	public:
		Message();
		Message(const std::string& topic, const char* body,int len);
		Message(const std::string& topic, const std::string& tags, const char* body,int len);
		Message(const std::string& topic, const std::string& tags,const std::string& keys, const char* body,int len);
		Message(const std::string& topic,
				const std::string& tags,
				const std::string& keys,
				const int	flag,
				const char* body,
				int len,
				bool waitStoreMsgOK);

		virtual ~Message();
		Message(const Message& other);
		Message& operator=(const Message& other);

		void clearProperty(const std::string& name);
		void putProperty(const std::string& name, const std::string& value);
		std::string getProperty(const std::string& name);

		std::string getTopic()const;
		void setTopic(const std::string& topic);

		std::string getTags();
		void setTags(const std::string& tags);

		std::string getKeys();
		void setKeys(const std::string& keys);
		void setKeys(const std::list<std::string> keys);

		int getDelayTimeLevel();
		void setDelayTimeLevel(int level);

		bool isWaitStoreMsgOK();
		void setWaitStoreMsgOK(bool waitStoreMsgOK);

		int getFlag();
		void setFlag(int flag);

		const char* getBody() const;
		int getBodyLen() const;
		void setBody(const char* body, int len);

		bool tryToCompress(int compressLevel);
		const char* getCompressBody() const;
		int getCompressBodyLen() const;

		std::map<std::string, std::string>& getProperties();
		void setProperties(const std::map<std::string, std::string>& properties);

		std::string toString() const;

	protected:
		void Init(const std::string& topic,
				  const std::string& tags,
				  const std::string& keys,
				  const int	flag,
				  const char* body,
				  int len,
				  bool waitStoreMsgOK);

	public:
		/**
		* 消息关键词，多个Key用KEY_SEPARATOR隔开（查询消息使用）
		*/
		static const std::string PROPERTY_KEYS;

		/**
		* 消息标签，只支持设置一个Tag（服务端消息过滤使用）
		*/
		static const std::string PROPERTY_TAGS;

		/**
		* 是否等待服务器将消息存储完毕再返回（可能是等待刷盘完成或者等待同步复制到其他服务器）
		*/
		static const std::string PROPERTY_WAIT_STORE_MSG_OK;

		/**
		* 消息延时投递时间级别，0表示不延时，大于0表示特定延时级别（具体级别在服务器端定义）
		*/
		static const std::string PROPERTY_DELAY_TIME_LEVEL;

		/**
		* 内部使用
		*/
		static const std::string PROPERTY_RETRY_TOPIC;
		static const std::string PROPERTY_REAL_TOPIC;
		static const std::string PROPERTY_REAL_QUEUE_ID;
		static const std::string PROPERTY_TRANSACTION_PREPARED;
		static const std::string PROPERTY_PRODUCER_GROUP;
		static const std::string PROPERTY_MIN_OFFSET;
		static const std::string PROPERTY_MAX_OFFSET;
		static const std::string PROPERTY_BUYER_ID;
		static const std::string PROPERTY_ORIGIN_MESSAGE_ID;
		static const std::string PROPERTY_TRANSFER_FLAG;
		static const std::string PROPERTY_CORRECTION_FLAG;
		static const std::string PROPERTY_MQ2_FLAG;
		static const std::string PROPERTY_RECONSUME_TIME;
		static const std::string KEY_SEPARATOR;
		static const std::string PROPERTY_MSG_REGION;
		static const std::string PROPERTY_UNIQ_CLIENT_MESSAGE_ID_KEYIDX;
		static const std::string PROPERTY_MAX_RECONSUME_TIMES;
		static const std::string PROPERTY_CONSUME_START_TIMESTAMP;

	private:
		std::string m_topic;///< 消息主题
		int m_flag;///< 消息标志，系统不做干预，完全由应用决定如何使用
		std::map<std::string, std::string> m_properties;///< 消息属性，系统有保留属性，应用也可以自定义属性

		char* m_body;///< 消息体
		int   m_bodyLen;///< 消息长度

		char* m_compressBody;///< 压缩消息体
		int   m_compressBodyLen;///< 压缩消息长度
	};
}

#endif
