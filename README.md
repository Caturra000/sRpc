# sRPC

## TL;DR

a RPC framework based on C++14

- 网络层基于我写的`mutty`
- 数据交互使用`JSON`，JSON库也是基于我写的`vsjson`
- 多线程、异步支持，且基于`future-promise`模型，超时可自己控制
- 使用超简单，不用写`stub`，不用`.*idl`，不用`generator`， 调用就完事了

## 快速使用

server端

```C++
#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcServer.h"
#include "Point.h"
using namespace mutty;
using namespace vsjson;

// POD支持
int add(int a, int b) {
    return a+b;
}

// RAII类支持
std::string append(std::string a, std::string b) {
    return a+b;
}

// 自定义类支持，见`Point.h`
Point pointAdd(Point a, Point b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}

int main() {
    Looper looper;
    RpcServer server(&looper, InetAddress("127.0.0.1", 23333));
    server.bind("add", add);
    server.bind("append", append);
    server.bind("pointAdd", pointAdd);
    server.start();
    looper.loop();
    return 0;
}
```

client端

```C++
#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcClient.h"
#include "Point.h"
using namespace mutty;
using namespace vsjson;

int main() {
    RpcClient client(InetAddress("127.0.0.1", 23333));
    auto connection = client.start();
    connection.get();
    std::future<int> addTest = client.call<int>("add", 1, 2);
    std::cout << addTest.get() << std::endl;
    std::future<std::string> appendTest = client.call<std::string>("append", "foo", "bar");
    std::future<Point> pointAddTest = client.call<Point>("pointAdd", Point{1,2,3}, Point{4,5,6});
    std::cout << appendTest.get() << std::endl;
    Point p = pointAddTest.get();
    std::cout << p.x << " " << p.y << " " << p.z << std::endl;
    return 0;
}
```

## TODO

- JSONRPC2.0兼容
- 异常处理