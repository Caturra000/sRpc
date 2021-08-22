#include <bits/stdc++.h>
#include "vsjson.hpp"
#include "RpcClient.h"
#include "Point.h"

nullptr_t fpp(int v) { return nullptr; }

int main() {
    fluent::InetAddress address {"127.0.0.1", 2333};
    srpc::RpcClient client {address};

    auto addFuture = client.call<int>("add", 1, 2)
        .then([](srpc::Try<int> result) {
            std::cout << "add:" << result.value() << std::endl;
            return nullptr;
        });

    // test normal case
    auto appendFuture = client.call<std::string>("append", "foo", "bar")
        .then([](srpc::Try<std::string> &&result) {
            std::cout << "string append: " << result.value() << std::endl;
            return nullptr;
        });

    // test RAII and convert class
    auto pointAddFuture = client.call<Point>("pointAdd", Point{1,2,3}, Point{4,5,6})
        .then([](srpc::Try<Point> &result) {
            auto value = result.value();
            std::cout << "point:" << value.x << ' ' << value.y << ' ' << value.z << std::endl;
            return value.x * value.y * value.z;
        }).then([](int p2v) {
            std::cout << "point to value: " << p2v << std::endl;
            return p2v;
        });

    // invalid params or method not found...
    auto errFut = client.call<int>("add", 123, 456, 789)
        .then([](srpc::Try<int> &&jojo) {
            std::cout << "test error" << std::endl;
            if(jojo.isException()) {
                try {
                    std::rethrow_exception(jojo.exception());
                } catch(const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            return "N/A";
        });

    auto alwaysCancel = [] { return false; };
    auto noReturnFuture = client.callWithListener<int>(alwaysCancel, "add", 1, 2)
        .then([](srpc::Try<int> result) {
            std::cout << "add2:" << result.value() << std::endl;
            return nullptr;
        });

    auto interceptor = [](vsjson::Json &json) {
        json[srpc::protocol::Field::params][1] = 100;
        return true;
    };

    auto noReturnFuture2 = client.callWith<int>(interceptor, alwaysCancel, "add", 1, 2)
        .then([](srpc::Try<int> result) {
            std::cout << "add2:" << result.value() << std::endl;
            return nullptr;
        });

    auto interceptFuture = client.callWithInterceptor<int>(interceptor, "add", 7, 8)
        // may print before latch stop this client
        .then([](srpc::Try<int> result) {
            std::cout << "add3:" << result.value() << std::endl;
            return nullptr;
        });

    auto latch = fluent::whenAll(client.looper(), addFuture, appendFuture, pointAddFuture, errFut)
        .then([&client](std::tuple<nullptr_t, nullptr_t, int, const char*> &&all) {
            std::cout  << std::get<3>(all) << std::endl;
            client.stop();
            return nullptr;
        });

    client.ready();
    client.run();
    return 0;
}