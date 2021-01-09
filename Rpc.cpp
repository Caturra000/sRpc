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

std::string append(std::string a, std::string b) {
    return a+b;
}

void minus(int a, int b) {
    
}

int main() try {

    RpcServer server;
    server.bind("add", add);
    server.bind("minus", minus);
    server.bind("append", append);
    auto json = server.netCall("append", Json::array("100", "200"));

    std::cout << json << std::endl;
    
    // RpcClient client(InetAddress("127.0.0.1", 23333));
    // auto future = client.call<int>("add", 1, 2);
    // auto result = future.get();
    //auto pr = std::make_shared<std::promise<int>>();
    // client.trySetValue(1, pr);
    
} catch (ErrnoException e) {
    std::cerr << e.errorCode() << " " << e.errorMessage() << std::endl;
}
