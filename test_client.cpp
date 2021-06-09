#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcClient.h"
#include "Point.h"
using namespace mutty;
using namespace vsjson;
using namespace srpc;

int main() {

    Looper looper;
    RpcClient client(&looper, {"127.0.0.1", 23333});
    auto thread = looper.loopAsync();
    client.start(Client::SyncPolicy::SYNC);
    auto cleanup = [&] {
        client.stop();
        looper.stop();
        thread.join();
    };
    Defer dtor {cleanup};

    // test invalid params
    std::future<int> addTest = client.call<int>("add", 1, 2, 3);
    try {
        std::cout << addTest.get() << std::endl;
    } catch(const protocol::Exception &e) {
        std::cerr << "catch: " << e.what() << std::endl;
    }

    // test method not found
    std::future<void> mock = client.call<void>("null", 1, 2, 3);
    try {
        mock.get();
    } catch(const protocol::Exception &e) {
        std::cerr << "catch: " << e.what() << std::endl;
    }

    // test normal case
    std::future<std::string> appendTest = client.call<std::string>("append", "foo", "bar");

    // test RAII and convert class
    std::future<Point> pointAddTest = client.call<Point>("pointAdd", Point{1,2,3}, Point{4,5,6});

    // test reverse async get() order
    Point p = pointAddTest.get();
    std::cout << p.x << " " << p.y << " " << p.z << std::endl;
    std::cout << appendTest.get() << std::endl;

    return 0;
}