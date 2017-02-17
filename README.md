Fork from [RocketMQ-Client4CPP](https://github.com/NDPMediaCorp/RocketMQ-Client4CPP)

[RocketMQ](https://github.com/alibaba/RocketMQ) C++ Client
===================

### 主要贡献者
* @[kangliqiang](https://github.com/kangliqiang)
* @[hooligan](79015166@qq.com)

### 目前现状
* 在原来的基础上修复了很多coredump，以及内存泄露（valgrind）问题，增加命名空间，补齐一些功能
* 去除对windows的支持，仅支持linux系统
* 目前支持发送消息，支持pull模式消费消息，支持push模式消费消息

### 发展规划
* 支持完整的事务消息
* 继续支持更多命令(比如broker反查运行信息)

### 已知BUG
* 消费过程中偶现某些队列停止消费的情况，查实是因为其中一个比较小的offset的消息一直没有从ProcessQueue对应的TreeMap中清除，导致最小最大offset差异过大，从而触发流控停止消费
* 生产消息过程中，偶现消息内容被损坏，这时脏消息已经写入broker，但是生产者会3s超时，但是因为send函数中本身还有2次重试，正常消息还是可以在重试过程中重写写入broker中，但是脏消息会被消费者消费到，目前通过在消费端检查消息格式暂时避免此问题


