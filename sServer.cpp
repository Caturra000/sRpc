#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
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