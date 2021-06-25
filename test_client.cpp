#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "RpcClient.h"
#include "RpcClientGroup.h"
#include "Point.h"
using namespace mutty;
using namespace vsjson;
using namespace srpc;

int main() {

    // test client
    { // scope guard

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

    } // scope guard


    // test client group
    { // scope guard

    Looper looper;
    RpcClientGroup group(&looper);
    group.emplace("127.0.0.1", 23333);
    group.emplace({"127.0.0.1", 23333});
    auto forEach = [&group](std::function<void(RpcClient&)> func) {
        for(auto &&node : group) {
            auto &&client = node.second;
            func(client);
        }
    };
    auto thread = looper.loopAsync();
    forEach([](RpcClient &client) {
        client.start(Client::SyncPolicy::SYNC);
    });
    auto cleanup = [&] {
        forEach([](RpcClient &client) { client.stop(); });
        looper.stop();
        thread.join();
    };
    mutty::Defer dtor {cleanup};
    int counter = 233;
    forEach([&counter](RpcClient &client) {
        auto addTest = client.call<int>("add", counter, counter);
        std::cout << addTest.get() << std::endl;
        counter <<= 1;
    });

    } // scope guard

    return 0;
}