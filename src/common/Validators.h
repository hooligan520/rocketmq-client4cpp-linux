/**
* Copyright (C) 2013 kangliqiang, kangliq@163.com
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
#ifndef __VALIDATORST_H__
#define  __VALIDATORST_H__

#include <string>

namespace rmq
{
    class MQClientException;
    class DefaultMQProducer;
    class Message;

    /**
    * 有效性检查公用类。
    *
    * @author manhong.yqd<jodie.yqd@gmail.com>
    * @since 2013-8-28
    */
    class Validators
    {
    public:
        /**
        * 通过正则表达式进行字符匹配
        *
        * @param origin
        * @param patternStr
        * @return
        */
        static bool regularExpressionMatcher(const std::string& origin, const std::string& patternStr);

        /**
        * 通过正则表达式查找匹配的字符
        *
        * @param origin
        * @param patternStr
        * @return
        */
        static std::string getGroupWithRegularExpression(const std::string& origin, const std::string& patternStr);

        /**
        * topic 有效性检查
        *
        * @param topic
        * @throws MQClientException
        */
        static void checkTopic(const std::string& topic);

        /**
        * group 有效性检查
        *
        * @param group
        * @throws MQClientException
        */
        static void checkGroup(const std::string& group);

        /**
        * message 有效性检查
        *
        * @param msg
        * @param int maxMessageSize
        * @throws MQClientException
        */
        static void checkMessage(const Message& msg, DefaultMQProducer* pDefaultMQProducer);

    public:
        static const std::string validPatternStr;
        static const size_t CHARACTER_MAX_LENGTH;
    };
}

#endif
