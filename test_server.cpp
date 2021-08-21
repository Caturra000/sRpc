#include <bits/stdc++.h>
#include "fluent.hpp"
#include "vsjson.hpp"
#include "RpcServer.h"
#include "Point.h"

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
    fluent::InetAddress address {"127.0.0.1", 2333};
    srpc::RpcServer server {address};
    server.bind("add", [](int a, int b) { return a+b; });
    server.bind("append", append);
    server.bind("pointAdd", pointAdd);
    server.ready();
    server.run();
    return 0;
}