#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcClient.h"
#include "point.h"
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