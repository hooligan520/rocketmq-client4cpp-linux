#include "Common.h"
#include "SendCallback.h"
#include "DefaultMQProducer.h"
using namespace rmq;


int print_screen = 0;
volatile long long g_cnt_succ = 0;
volatile long long g_cnt_fail = 0;


void Usage(const char* program)
{
    printf("Usage:%s ip:port [-g group] [-t topic] [-n count] [-s size]\n", program);
    printf("\t -g group\n");
    printf("\t -t topic\n");
    printf("\t -n message count\n");
    printf("\t -s message size \n");
}


class SampleSendCallback : public SendCallback {
public:
    SampleSendCallback()
    {
    }

    virtual ~SampleSendCallback()
    {
        std::cout << "~Ctor invoked" << std::endl;
    }

    void onSuccess(SendResult& sendResult)
    {
        __sync_fetch_and_add(&g_cnt_succ, 1);

        if (print_screen)
        {
            printf("result:{sendStatus=%d,msgId=%s,messageQueue=[topic=%s,brokerName=%s,queueId=%d],queueOffset=%d}\n",
                sendResult.getSendStatus(), sendResult.getMsgId().c_str(),
                sendResult.getMessageQueue().getTopic().c_str(),
                sendResult.getMessageQueue().getBrokerName().c_str(),
                sendResult.getMessageQueue().getQueueId(),
                sendResult.getQueueOffset());
        }
    }

    void onException(MQException& e)
    {
        __sync_fetch_and_add(&g_cnt_fail, 1);

        printf("Message delivery failed, cause: %s\n", e.what());
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2)
    {
        Usage(argv[0]);
        return 0;
    }

    std::string namesrv = argv[1];
    std::string group = "pg_test_group";
    std::string topic = "topic_test";
    int size = 32;
    int count = 1000;

    for (int i=2; i< argc; i++)
    {
        if (strcmp(argv[i],"-g")==0)
        {
            if (i+1 < argc)
            {
                group = argv[i+1];
                i++;
            }
            else
            {
                Usage(argv[0]);
                return 0;
            }
        }
        else if (strcmp(argv[i],"-t")==0)
        {
            if (i+1 < argc)
            {
                topic = argv[i+1];
                i++;
            }
            else
            {
                Usage(argv[0]);
                return 0;
            }
        }
        else if (strcmp(argv[i],"-n")==0)
        {
            if (i+1 < argc)
            {
                count = atoi(argv[i+1]);
                i++;
            }
            else
            {
                Usage(argv[0]);
                return 0;
            }
        }
        else if (strcmp(argv[i],"-s")==0)
        {
            if (i+1 < argc)
            {
                size = atoi(argv[i+1]);
                i++;
            }
            else
            {
                Usage(argv[0]);
                return 0;
            }
        }
        else
        {
            Usage(argv[0]);
            return 0;
        }
    }

    // 初始化client api日志，此处非必要，需要对api进行调试才需要进行初始化，可以考虑注释
    // 这里默认只打印警告、错误日志，日志会按天滚动，如果需要修改日志级别，请设置一下环境变量，export ROCKETMQ_LOGLEVEL=日志级别
    // 日志级别如下:
    //  0   - 关闭日志
    //  1   - 写错误 日志
    //  2   - 写错误,警告 日志
    //  3   - 写错误,警告,信息 日志
    //  4   - 写错误,警告,信息,调试 日志
    RocketMQUtil::initLog("/tmp/rocketmq_producer.log");

    // 初始化RocketMQ生产者，传入生产组名称
    RMQ_DEBUG("producer.new: %s", "pg_CppClient");
    DefaultMQProducer producer("pg_CppClient");

    // 设置MQ的NameServer地址
    RMQ_DEBUG("producer.setNamesrvAddr: %s", namesrv.c_str());
    producer.setNamesrvAddr(namesrv);

    // 启动生产者
    RMQ_DEBUG("producer.start");
    producer.start();

    std::string tags[] = { "TagA", "TagB", "TagC", "TagD", "TagE" };
    int nNow = time(NULL);
    char key[64];
    char value[1024];

    std::string str;
    for (int i = 0; i < size; i += 8)
    {
        str.append("hello baby");
    }

    TimeCount tcTotal;
    tcTotal.begin();

    SampleSendCallback* ptrSendCallback = new SampleSendCallback();
    for (int i = 0; i < count; i++)
    {
        try
        {
            snprintf(key, sizeof(key), "KEY_%d_%d", nNow, i);
            snprintf(value, sizeof(value), "%011d_%s", i, str.c_str());
            Message msg(topic,// topic
                tags[i % 5],// tag
                key,// key
                value,// body
                strlen(value)+1
            );

            // 异步生产消息
            producer.send(msg, ptrSendCallback);
        }
        catch (MQClientException& e)
        {
            std::cout << e << std::endl;
            __sync_fetch_and_add(&g_cnt_fail, 1);
            MyUtil::msleep(3000);
        }
    }

    while (1)
    {
        if ((g_cnt_succ + g_cnt_fail)  >= count)
        {
            break;
        }
    }

    tcTotal.end();

    printf("statsics: succ=%d, fail=%d, total_cost=%ds, tps=%d, avg=%dms\n",
        g_cnt_succ, g_cnt_fail, tcTotal.countSec(),
        (int)((double)count/(tcTotal.countMsec()/1000)), tcTotal.countMsec()/count);

    // 停止生产者
    producer.shutdown();

    return 0;
}

