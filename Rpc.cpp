#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "RpcClient.h"
using namespace mutty;
using namespace vsjson;

class RpcService {
public:
    template <typename F>
    void bind(const std::string &name, const F &func) {
        _table[name] = CallProxy<F>(func);
    }

    template <typename Ret, typename ...Args>
    Ret call(const std::string &name, Args ...args) { // 不应是args，应该是json

        // codec解包json，得到Json
        // 构造char stream[65536];
        // 根据json的内容reinterpret到stream中

        // TODO stream这一步其实很多余
        // 不如直接传入json
        // 需要修改table类型
        // make过程稍微改一下


        // HARD CODE
        char buf[1234]{};
        return _table[name](buf, sizeof(int)*2).as<Ret>();
    }
private:
    std::map<std::string, std::function<Json(const char*, size_t)>> _table;
};


int add(int a, int b) {
    return a+b;
}

void minus(int a, int b) {
    
}




class RpcServer {

};


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
