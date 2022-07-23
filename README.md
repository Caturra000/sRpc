# sRPC

## 注意

这个RPC库搁置长达2年了，历史使命已完成，目前已经被[tRPC](https://github.com/Caturra000/tRPC)所替代

并且下面的README已过期，后续也基本不会更新

## TL;DR

a RPC framework based on C++14

- 网络层基于我写的[mutty](https://github.com/Caturra000/mutty)和[fluent](https://github.com/Caturra000/FluentNet/)
- `JSON-RPC 2.0`兼容，JSON库也是基于我写的[vsjson](https://github.com/Caturra000/vsjson/)
- 多线程、异步支持，基于`future-promise`模型
- 不需要额外的idl，也不需要code generator，只有C++
- header only，开箱即用

## 快速使用

server端

```C++
#include <bits/stdc++.h>
#include "RpcServer.h"
#include "Point.h"

std::string append(std::string a, std::string b) {
    return a+b;
}

Point pointAdd(Point a, Point b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}

int main() {
    mutty::Looper looper;
    srpc::RpcServer server(&looper, {"127.0.0.1", 23333});

    // lambda
    server.bind("add", [](int a, int b) { return a+b; });

    // std::function
    std::function<std::string(std::string, std::string)> func = append;
    server.bind("append", func);

    // simple function
    server.bind("pointAdd", pointAdd);

    server.start();
    looper.loop();
    return 0;
}
```

client端

```C++
#include <bits/stdc++.h>
#include "RpcClient.h"
#include "Point.h"

int main() {
    mutty::Looper looper;
    srpc::RpcClient client(&looper, {"127.0.0.1", 23333});
    auto thread = looper.loopAsync();
    client.start(srpc::RpcClient::SyncPolicy::SYNC);
    auto cleanup = [&] {
        client.stop();
        looper.stop();
        thread.join();
    };
    mutty::Defer dtor {cleanup};

    std::future<int> addTest = client.call<int>("add", 1, 2);
    std::cout << addTest.get() << std::endl;

    std::future<std::string> appendTest = client.call<std::string>("append", "foo", "bar");
    std::future<Point> pointAddTest = client.call<Point>("pointAdd", Point{1,2,3}, Point{4,5,6});

    Point p = pointAddTest.get();
    std::cout << p.x << " " << p.y << " " << p.z << std::endl;
    std::cout << appendTest.get() << std::endl;

    return 0;
}
```

要点：

1. server通过`bind`绑定远程调用函数，client通过`call`获取响应
2. `bind`支持函数指针，`std::function`以及`lambda`（自定义functor理论上支持）
3. `call`是异步的，相当于提供一个`promise`，返回对应`std::future`
4. 参数支持任意自定义类型，见`point.h`实现示例（但不建议使用数组和裸指针传入）
5. 对于异常情况，见`test_client.cpp`使用`srpc::protocol::Exception`处理示例
6. 直接使用`std::future::get`是会阻塞当前线程的（但不影响client），见[cppreference / std::future](https://en.cppreference.com/w/cpp/thread/future)了解更多API
7. `dtor`将会在`main`结束时调用，非必要使用`mutty::Defer`，但个人推荐使用

## TODO

拦截器

类型支持改用非侵入式的特化实现
