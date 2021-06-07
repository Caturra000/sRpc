#ifndef __SRPC_RPC_CLIENT_H__
#define __SRPC_RPC_CLIENT_H__
#include "mutty.hpp"
#include "vsjson.hpp"
#include "Codec.h"
#include "Protocol.h"
namespace srpc {

class RpcClient: private mutty::NonCopyable {
public:
    template <typename T, typename ...Args> // PODs
    std::future<T> call(const std::string &func, Args &&...args);

    std::future<bool> start();
    void join() { _client.stopLatch(); }

    RpcClient(const mutty::InetAddress &serverAddress);

private:
    template <typename T>
    void trySetValue(int id, std::shared_ptr<std::promise<T>> promise);
    void trySetValue(int id, std::shared_ptr<std::promise<void>> promise);

    template <typename ...Args>
    void makeRequestImpl(vsjson::Json &json, Args &&...args) {}
    template <typename Arg, typename ...Args>
    void makeRequestImpl(vsjson::Json &json, Arg &&arg, Args &&...args);
    template <typename ...Args>
    vsjson::Json makeRequest(const std::string &method, Args &&...args);

private:
    mutty::AsyncLooperContainer _looperContainer;
    mutty::Client _client;
    int _idGen;
    std::map<int, vsjson::Json> _records;
    Codec codec;
    std::promise<bool> _connectPromise;
};

template <typename T, typename ...Args> // PODs
inline std::future<T> RpcClient::call(const std::string &func, Args &&...args) {
    auto request = std::make_shared<vsjson::Json>(makeRequest(func, std::forward<Args>(args)...));
    auto promise = std::make_shared<std::promise<T>>();
    _client.async([this, request, promise] {
        int id = ++_idGen;
        (*request)[Protocol::id] = id;
        std::string dump = request->dump();
        uint32_t length = dump.length();
        uint32_t beLength = htonl(length);
        // TODO 实现类似folly的then
        _client.send(&beLength, sizeof(uint32_t));
        _client.send(dump.c_str(), length);
        _client.async([=] {
            trySetValue(id, promise);
        });
    });
    return promise->get_future();
}

inline std::future<bool> RpcClient::start() {
    _client.start();
    return _connectPromise.get_future();
}

inline RpcClient::RpcClient(const mutty::InetAddress &serverAddress)
    : _client(_looperContainer.get(), serverAddress),
        _idGen(mutty::random<int>() & 65535) {
    _client.onConnect([this] {
        _connectPromise.set_value(true);
    });
    _client.onMessage([this](mutty::TcpContext *ctx) {
        while(codec.verify(ctx->inputBuffer)) {
            vsjson::Json response = codec.decode(ctx->inputBuffer);
            int id = response[Protocol::id].as<int>();
            _records[id] = std::move(response[Protocol::result]);
        }
    });
}

template <typename T>
inline void RpcClient::trySetValue(int id, std::shared_ptr<std::promise<T>> promise) {
    auto iter = _records.find(id);
    if(iter != _records.end()) {
        promise->set_value(std::move(iter->second).to<T>());
        _records.erase(iter);
    } else {
        _client.async([=] {
            trySetValue(id, promise);
        }); // retry
    }
}

inline void RpcClient::trySetValue(int id, std::shared_ptr<std::promise<void>> promise) {
    auto iter = _records.find(id);
    if(iter != _records.end()) {
        promise->set_value();
        _records.erase(iter);
    } else {
        _client.async([=] {
            trySetValue(id, promise);
        }); // retry
    }
}

template <typename Arg, typename ...Args>
inline void RpcClient::makeRequestImpl(vsjson::Json &json, Arg &&arg, Args &&...args) {
    json.append(std::forward<Arg>(arg));
    makeRequestImpl(json, std::forward<Args>(args)...);
}

template <typename ...Args>
inline vsjson::Json RpcClient::makeRequest(const std::string &method, Args &&...args) {
    vsjson::Json json =
    {
        {Protocol::jsonrpc, Protocol::version},
        {Protocol::method, method},
        {Protocol::params, vsjson::Json::array()}
    };
    vsjson::Json &argsJson = json[Protocol::params];
    makeRequestImpl(argsJson, std::forward<Args>(args)...);
    return json;
}

} // srpc
#endif