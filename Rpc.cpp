#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "RpcClient.h"
#include "RpcServer.h"
using namespace mutty;
using namespace vsjson;



int add(int a, int b) {
    return a+b;
}

void minus(int a, int b) {
    
}

int main() try {
    // std::function<int(int, int)> func = add;

    // CallProxy<decltype(add)> cf = add;
    // CallProxy<decltype(func)> cf_func = func;
    // CallProxy<decltype(minus)> mn = minus;
    // char buf[sizeof(1)+sizeof(2)];
    // cf(buf, sizeof(1)+sizeof(2));
    // cf_func(buf, sizeof(1)+sizeof(2));;
    // std::function<void(const char*, size_t)> f = CallProxy<decltype(add)>(add);
    // Service service;
    // service.bind("add", add);
    // service.bind("min", minus);
    // Test t;
    // auto memberFunc = &Test::add;
    // auto lambdaFunc = [&](int,int)->int { return 1; };

    auto lam = [](int n) {
        std::function<int(int)> lam2;
        lam2 = [&] (int n) {
            return n == 1 ? 1 : n + lam2(n-1);
        };
        return lam2(n);
        
    };
    std::cout << lam(3) << std::endl;

    RpcService rpc;
    rpc.bind("add", add);
    std::function<int(int, int)> func = add;
    rpc.bind("func", func);
    auto r = rpc.call<int>("add", 1, 2);
    std::cout << r;
    std::cout << std::endl;
    
    RpcClient client(InetAddress("127.0.0.1", 23333));
    auto future = client.call<int>("add", 1, 2);
    auto result = future.get();
    //auto pr = std::make_shared<std::promise<int>>();
    // client.trySetValue(1, pr);
    
} catch (ErrnoException e) {
    std::cerr << e.errorCode() << " " << e.errorMessage() << std::endl;
}
