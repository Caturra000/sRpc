#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "RpcClient.h"
#include "RpcServer.h"
using namespace mutty;
using namespace vsjson;


struct Point {
    int x, y, z;
    operator Json() { return Json::array(x, y, z); }
    Point(int x, int y, int z): x(x), y(y), z(z) {}
    Point(const std::vector<vsjson::Json> &array)
        : x(array[0].to<int>()),
          y(array[1].to<int>()),
          z(array[2].to<int>()) {}
    Point() = default;
};


int add(int a, int b) {
    return a+b;
}

std::string append(std::string a, std::string b) {
    return a+b;
}

Point pointAdd(Point a, Point b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}

void minus(int a, int b) {
    
}

int main() try {

    RpcServer server;
    server.bind("add", add);
    server.bind("minus", minus);
    server.bind("append", append);
    server.bind("p", pointAdd);
    auto json = server.netCall("p", Json::array(Point{1,2,3}, Point{4,5,6}));
    auto point = json.to<Point>();
    auto json2 = server.netCall("append", Json::array("abc", "def"));
    std::cout << point.x << std::endl;
    std::cout << json2 << std::endl;
    
    // RpcClient client(InetAddress("127.0.0.1", 23333));
    // auto future = client.call<int>("add", 1, 2);
    // auto result = future.get();
    //auto pr = std::make_shared<std::promise<int>>();
    // client.trySetValue(1, pr);
    
} catch (ErrnoException e) {
    std::cerr << e.errorCode() << " " << e.errorMessage() << std::endl;
}
