#ifndef __RPC_RPC_CLIENT_H__
#define __RPC_RPC_CLIENT_H__
#include "mutty.hpp"
#include "vsjson.hpp"
#include "Codec.h"
#include "RpcField.h"
using namespace mutty;
using namespace vsjson;
class RpcClient {
public:
    template <typename T>
    std::enable_if_t<!std::is_same<T,void>::value> 
    trySetValue(int token, std::shared_ptr<std::promise<T>> promise) {
        auto iter = _records.find(token);
        if(iter != _records.end()) {
            promise->set_value(std::move(iter->second).to<T>());
            _records.erase(iter);
        } else {
            _client.startTransaction([=] {
                trySetValue(token, promise);
            }).commit(); // retry
        }
    }

    template <typename T>
    std::enable_if_t<std::is_same<T,void>::value> 
    trySetValue(int token, std::shared_ptr<std::promise<T>> promise) {
        auto iter = _records.find(token);
        if(iter != _records.end()) {
            promise->set_value();
            _records.erase(iter);
        } else {
            _client.startTransaction([=] {
                trySetValue(token, promise);
            }).commit(); // retry
        }
    }

    template <typename T, typename ...Args> // PODs
    std::future<T> call(const std::string &func, Args &&...args) {
        auto request = std::make_shared<Json>(makeRequest(func, std::forward<Args>(args)...));
        auto promise = std::make_shared<std::promise<T>>();
        _client.startTransaction([this, request, promise] {
            int token = ++_idGen;
            (*request)[RpcField::TOKEN] = token;
            std::string dump = request->dump();
            uint32_t length = dump.length();
            uint32_t beLength = htonl(length);
            // TODO 实现类似folly的then
            _client.send(&beLength, sizeof(uint32_t));
            _client.send(dump.c_str(), length);
            _client.startTransaction([=] {
                trySetValue(token, promise);
            }).commit();
        }).commit();
        return promise->get_future();
    }

    std::future<bool> start() {
        _client.start();
        return _connectPromise.get_future();
    }

    void join() { _client.join(); }

    RpcClient(const InetAddress &serverAddress)
        : _client(_looperContainer.get(), serverAddress),
          _idGen(random<int>() & 65535) {
        _client.onConnect([this] {
            _connectPromise.set_value(true);
        });
        _client.onMessage([this](TcpContext *ctx) {
            while(codec.verify(ctx->inputBuffer)) {
                Json response = codec.decode(ctx->inputBuffer);
                int token = response[RpcField::TOKEN].as<int>();
                _records[token] = std::move(response[RpcField::RETURN]);
            }
        });
    }

private:
    template <typename ...Args>
    void makeRequestImpl(Json &json, Args &&...args) {
    }

    template <typename Arg, typename ...Args>
    void makeRequestImpl(Json &json, Arg &&arg, Args &&...args) {
        json.append(std::forward<Arg>(arg));
        makeRequestImpl(json, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    Json makeRequest(const std::string &name, Args &&...args) {
        Json json = 
        {
            {RpcField::NAME, name},
            {RpcField::ARGS, Json::array()}
        };
        Json &argsJson = json[RpcField::ARGS];
        makeRequestImpl(argsJson, std::forward<Args>(args)...);
        return json;
    }

    AsyncLooperContainer _looperContainer;
    Client _client;
    int _idGen;
    std::map<int, Json> _records;
    Codec codec;
    std::promise<bool> _connectPromise;
};
#endif