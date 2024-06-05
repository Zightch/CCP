# CCP协议
一个基于UDP实现的面向连接可靠传输协议

## 协议文档请看[这里](protocol.md)

## 为什么使用CCP
* 传输更高效, 更低的延迟, UDP特性, 数据包可以很快抵达对方主机
* 在连接成功之前不会占用连接数量, 避免类似TCP的SYN攻击
* 有数据传输应答和超时重传机制, 以及数据包ID标识, 以确保数据包不混乱以及一定能到达对方主机
* 用更严苛和简单的规则来限制通讯, 有任何问题立即关闭连接, 节省资源占用
* 更灵活的流量控制, 允许自定义窗口大小以及单个数据包长度
* 每10秒(可自定义)进行心跳, 避免TCP长时间不活跃导致连接不稳定
* 本协议基于数据包传输, UDP特性, 避免TCP流的数据边界不明确问题(粘包)
* 允许发送不安全的数据包, 及数据包不需要确认对方收到, 可以立即发送
* 支持大数据包的高效传输, 通过链表包机制自动分割和重组大于常规MTU的数据, 可以传输文件
* 是一个轻量级协议, 可以作为嵌入式使用

综上所述, CCP通过结合UDP的高效与自定义的可靠传输机制  
旨在解决一些TCP的缺点并且构造一个简单可靠的传输  
提供了一个既灵活又高效, 且易于实现的通讯协议选项  
尤其适合那些需要优化延迟, 控制通讯流程细节或在特定环境下运行的应用  
例如游戏, 音频视频实时传输

## 许可
[MIT](LICENSE)

## 实现
本项目包含一个QT实现版本的CCP协议  
当然也包括图形化界面, QT版本为6.7.0以上  
开发者们可以以此为参考
