#ifndef __SRPC_RPC_CLIENT_H__
#define __SRPC_RPC_CLIENT_H__
#include "fluent.hpp"
#include "vsjson.hpp"
#include "Codec.h"
#include "Protocol.h"
#include "Try.h"
namespace srpc {

class RpcClient {
public:
    template <typename T, typename ...Args> // PODs
    fluent::Future<Try<T>> call(const std::string &func, Args &&...args);
    template <typename T, size_t N, typename ...Args>
    fluent::Future<Try<T>> call(const std::string &func, Args &&...args);

    void ready();

    void run() { _client.run(); }
    void batch() { _client.batch(); }
    void stop() { _client.stop(); }

    fluent::Looper* looper() { return _client.looper(); }

    RpcClient(const fluent::InetAddress &address);
    ~RpcClient() = default;
    RpcClient(const RpcClient&) = delete;
    RpcClient(RpcClient&&) = default;
    RpcClient& operator=(const RpcClient&) = delete;
    RpcClient& operator=(RpcClient&&) = default;

private:
    template <typename ...Args>
    void makeRequestImpl(vsjson::Json &json, Args &&...args) {}
    template <typename Arg, typename ...Args>
    void makeRequestImpl(vsjson::Json &json, Arg &&arg, Args &&...args);
    template <typename ...Args>
    vsjson::Json makeRequest(const std::string &method, Args &&...args);

private:
    fluent::Client _client;
    fluent::InetAddress _address; // server
    fluent::Context *_context; // from _client
    int _idGen;
    std::map<int, vsjson::Json> _records;
    Codec codec;
};

inline void RpcClient::ready() {
    auto fut = _client.connect(_address)
        .then([this](fluent::Context *context) {
            _context = context;
            return nullptr;
        });
}

template <typename T, typename ...Args> // PODs
inline fluent::Future<Try<T>> RpcClient::call(const std::string &func, Args &&...args) {
    auto requestPtr = std::make_shared<vsjson::Json>(makeRequest(func, std::forward<Args>(args)...));
    return fluent::makeFuture(_client.looper(), nullptr)
        .poll([this](nullptr_t) {
            return _context != nullptr;
        })
        .then([request = std::move(requestPtr), this](nullptr_t) {
            auto context = _context;
            int id = ++_idGen;
            (*request)[protocol::Field::id] = id;
            std::string dump = request->dump();
            uint32_t length = dump.length();
            uint32_t beLength = htonl(length);
            context->send(&beLength, sizeof(uint32_t));
            context->send(dump.c_str(), length);
            return std::make_tuple(id, vsjson::Json());
        })
        .poll([this](std::tuple<int, vsjson::Json> &&info) {
            auto context = _context;
            auto &buf = context->input;
            int id = std::get<0>(info);
            auto &result = std::get<1>(info);
            if(codec.verify(buf)) {
                auto response = codec.decode(buf);
                if(response[protocol::Field::id].to<int>() != id) {
                    _records[id] = std::move(response);
                } else {
                    result = std::move(response);
                    return true;
                }
            }
            auto iter = _records.find(id);
            if(iter != _records.end()) {
                result = std::move(iter->second);
                _records.erase(iter);
                return true;
            }
            return false;
        })
        .then([this](std::tuple<int, vsjson::Json> &&info) {
            auto &response = std::get<1>(info);
            if(!response.contains(protocol::Field::error)) {
                return Try<T>(std::move(response[protocol::Field::result].to<T>()));
            }
            auto errObj = std::move(response[protocol::Field::error]);
            return errObj.contains(protocol::Field::message) ?
                Try<T>(std::make_exception_ptr(protocol::Exception(
                    errObj[protocol::Field::code].to<int>(),
                    errObj[protocol::Field::message].to<std::string>())))
              : Try<T>(std::make_exception_ptr(protocol::Exception(
                    errObj[protocol::Field::code].to<int>())));
        });
}

template <typename T, size_t N, typename ...Args>
inline fluent::Future<Try<T>> RpcClient::call(const std::string &func, Args &&...args) {
    static_assert(N == sizeof...(Args), "invalid params");
    return call<T>(func, std::forward<Args>(args)...);
}

inline RpcClient::RpcClient(const fluent::InetAddress &address)
    : _client(),
      _address(address),
      _idGen(::random() & 65535),
      _context(nullptr) {}

template <typename Arg, typename ...Args>
inline void RpcClient::makeRequestImpl(vsjson::Json &json, Arg &&arg, Args &&...args) {
    json.append(std::forward<Arg>(arg));
    makeRequestImpl(json, std::forward<Args>(args)...);
}

template <typename ...Args>
inline vsjson::Json RpcClient::makeRequest(const std::string &method, Args &&...args) {
    vsjson::Json json =
    {
        {protocol::Field::jsonrpc, protocol::Attribute::version},
        {protocol::Field::method, method},
        {protocol::Field::params, vsjson::Json::array()}
    };
    vsjson::Json &argsJson = json[protocol::Field::params];
    makeRequestImpl(argsJson, std::forward<Args>(args)...);
    return json;
}

} // srpc
#endif