# Mutty

## TL;DR

`Mutty`是一个基于`C++14`和`Reactor`模型开发的基础网络库，满足Modern C++和高性能的需求，拥有简洁的接口，适用于`TCP`通信，并且header-only

## 快速使用

这是一个`echo server/client`的demo，可以了解到`mutty`的大概使用流程

```C++
#include <bits/stdc++.h>
#include "mutty.hpp"
using namespace mutty;
int main() {
    Looper looper;
    Server server(&looper, InetAddress("127.0.0.1", 2333));

    server.onConnect([] {
        std::cout << "connection created." << std::endl;
    });

    server.onMessage([](TcpContext *ctx) {
        auto &buf = ctx->inputBuffer;
        std::cout << buf << std::endl;
        ctx->send(buf.readBuffer(), buf.unread());
        buf.read(buf.unread());
    });

    server.onClose([] {
        std::cout << "connection close." << std::endl;
    });
    
    server.start();
    looper.loop();
    return 0;
}
```



```C++
#include <bits/stdc++.h>
#include "mutty.hpp"
using namespace mutty;
void print(const char *message) { std::cout << message << std::endl; }
int main() {
    AsyncLooperContainer container;
    Client client(container.get(), InetAddress("127.0.0.1", 2333));
    
    client.onConnect(print, "[client] connected.");
    client.onClose([&] {
        print("[client] closed.");
        auto looper = container.get();
        looper->stop();
    });

    client.start();
    FastIo<65536> io;
    while(auto result = io.getline()) {
        client.startTransaction([=, &client] {
            client.send(result.buf, result.len);
        }).then([&] {
            print("[client] result has been sent to buffer.");
        }).commit();
    }
    return 0;
}
```



## 文件分布

`src`包含所有实现代码，介绍会以bottom-up的形式进行

`utils`是一个基础库，实现了如下的功能模块，可独立于`mutty`项目使用：

- `Callable`：可调用对象的抽象，也是`mutty`中所有`callback`类型的实现
- `Defer`：实现类似于`GO`语言中的`defer`语义
- `Timestamp`：提高时间接口舒适度的类型别名，可以实现`1s / 5min / 12ns`的表示形式
- `Object / Exchanger`：一个`std::any`的`copyable / noncopyable`简化版
- `ThreadPool`：提供简单的通用线程池
- `Algorithms / StringUtils`：实现一些二进制操作、随机数生成、字符串分割之类的便利功能
- `Pointer`：实现一个支持`move`语义的原生指针封装
- `Compat`：对于`C++11`标准缺失的接口会在这里以`namespace cpp11`提供
- `TypeTraits`：类型萃取的封装，主要给`SFINAE`提供支持

`net`是关于`socket`的一些最简单的封装，原则是与`unix api`兼容：

- `Socket`：`socket`文件描述符的简单抽象，只负责管理生命周期
- `InetAddress`：`sockaddr`族的简单封装，方便快速构造和处理大小端问题

`base`是`mutty`项目底层模块的划分

- `handler/`：实现`Message-MessageQueue-Handler`的派发机制
- `buffer/`：提供可缓存、可减少内存碎片的`Buffer`实现
- `timer/`：一个通用、简洁的定时器，基于堆的实现
- `context/`：负责资源的上下文管理，也处理IO多路复用的交互，可暴露于回调接口

`core`是涉及`Reactor`的实现模块，依赖于`base`模块

- `AcceptHandler / AcceptContext`：`accept`过程及其回调接口的封装
- `TcpHandler / TcpContext / ConnectionPool`：`Tcp`连接的处理、上下文、全局管理的抽象
- `Looper / LooperPool`：`Reactor`的入口
- `Multiplexer`：IO多路复用实现的模块，只提供`epoll`封装

`app`是直接使用的模块

- `Server`
- `Client`

## 实现细节

`mutty`的实现是极其轻量级的，但不意味着写的随便

### 接口提供

对外接口的提供为了尽可能简洁，使用了诸如`Builder`、`user-define literal`、`SFINAE`、`Policy`的技巧

对于`server`和`client`，扩展功能并不需要继承一个类来重载实现，可以在对象的层面上直接注册回调，并提供多种选择

一个`server`或者`client`提供以下基本回调接口

```C++
server.onConnect();
server.onMessage();
server.onWriteComplete();
server.onClose();
```

以`client`为例，每种回调注册都可以接受无状态、有上下文、含生命周期管理的上下文三种回调接口

一般配合`lambda`的捕获，功能足够强大

```C++
#include <bits/stdc++.h>
#include "app/Client.h"

void print(const char *name, int num, double rate) {
    std::cout << name << " " << num << " " << rate << std::endl;
}

int main() {
    Looper looper;
    Client client(&looper, InetAddress("127.0.0.1", 2333));
    // 无状态
    client.onConnect(print, "jojo", 1, 0.3);
    client.onConnect([] { abort(); });
    // 上下文
    client.onConnect([&](TcpContext *ctx) {
        // ...
    });
    // 生命周期安全
    client.onConnect([](std::weak_ptr<TcpContext> context) {
        if(auto ctx = context.lock()) {
            // ...
        }
    });
    
    client.start();
    looper.loop();
}
```

另外，定时器的使用也是极其轻松的

```C++
TcpContext *ctx; 
// ...
timer.runAfter(now() + 5s).atMost(3).per(500ms)
    .with([ctx] { ctx->send("tick-tock"); });
```

内部接口为了实现上的舒适也造了少量轮子，如`Callable`和`Defer`



### 线程安全

线程安全参考了`muduo`的`runInLoop`做法，但是这里并不是通过一个封装`std::queue<std::function>`的容器来实现

`mutty`的线程安全分为两个方法，

一种是`handler`机制，用于固定发生的事件回调，

通过`Message`生成消息，放入`MessageQueue`派发到合适的`Handler`来实现调度到`Loop`线程

另一种是直接基于`Timer`，通过`Timer::ResultSet`在`Loop`线程获取合适的事件

这两种都用到`swap`的技巧来减小锁的粒度

### 缓存和碎片处理

// 有空写



## TODO

- LOG
- test
- Codec-demo



## MORE

[十行以内实现一个defer](http://www.caturra.cc/2020/09/22/十行以内实现一个defer/)

[通过滑动窗口来优化vector](http://www.caturra.cc/2020/11/08/通过滑动窗口来优化vector/)

[设计一个引起舒适的回调接口](http://www.caturra.cc/2020/11/19/设计一个引起舒适的回调接口/)

[定时器的简单讨论](http://www.caturra.cc/2020/12/07/定时器的简单讨论/)