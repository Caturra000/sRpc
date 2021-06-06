#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcServer.h"
#include "Point.h"
using namespace mutty;
using namespace vsjson;

// int add(int a, int b) {
//     return a+b;
// }

std::string append(std::string a, std::string b) {
    return a+b;
}

Point pointAdd(Point a, Point b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
int main() {
    Looper looper;
    RpcServer server(&looper, InetAddress("127.0.0.1", 23333));
    server.bind("add", [](int a, int b) { return a+b; });
    server.bind("append", append);
    server.bind("pointAdd", pointAdd);
    server.start();
    looper.loop();
    return 0;
}