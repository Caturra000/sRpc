#ifndef __RPC_RPC_CLIENT_H__
#define __RPC_RPC_CLIENT_H__
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
using namespace mutty;
using namespace vsjson;
class RpcClient {
public:
    template <typename T>
    void trySetValue(int token, std::shared_ptr<std::promise<T>> promise) {
        auto iter = records.find(token);
        if(iter != records.end()) {
            promise->set_value(std::move(iter->second).to<T>());
            records.erase(iter);
        } else {
            client.startTransaction([=] {
                trySetValue(token, promise);
            }).commit(); // retry
        }
    }

    template <typename T, typename ...Args> // PODs
    std::future<T> call(const std::string &func, Args &&...args) {
        auto request = std::make_shared<Json>(makeRequest(func, std::forward<Args>(args)...));
        auto promise = std::make_shared<std::promise<T>>();
        client.startTransaction([this, request, promise] {
            int token = ++idGen;
            (*request)["token"] = token;
            std::string dump = request->dump();
            uint32_t length = dump.length();
            // TODO 实现类似folly的then
            client.startTransaction([this, length, data = std::move(dump)] {
                client.send(&length, sizeof(uint32_t));
                client.send(data.c_str(), length);
            }).commit();
            client.startTransaction([=] {
                trySetValue(token, promise);
            }).commit();
        }).commit();
        return promise->get_future();
    }

    void start() { client.start(); }

    RpcClient(const InetAddress &serverAddress)
        : client(looperContainer.get(), serverAddress),
          idGen(random<int>() & 65535) {
        client.onMessage([this](TcpContext *ctx) {
            if(codec.verify(ctx->inputBuffer)) {
                Json response = codec.decode(ctx->inputBuffer);
                int token = response["token"].as<int>();
                records[token] = std::move(response["ret"]);
            }
        });
    }

private:
    template <typename ...Args>
    void makeRequestImpl(Json &json, Args &&...args) {
    }

    template <typename Arg, typename ...Args>
    void createImpl(Json &json, Arg &&arg, Args &&...args) {
        json.append(std::forward<Arg>(arg));
        createImpl(json, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    Json makeRequest(const std::string &func, Args &&...args) {
        Json json = 
        {
            {"func", func},
            {"args", Json::array()}
        };
        Json &argsJson = json["args"];
        makeRequestImpl(argsJson, std::forward<Args>(args)...);
        return json;
    }

    AsyncLooperContainer looperContainer;
    Client client;
    int idGen;
    std::map<int, Json> records;
    Codec codec;
};
#endif