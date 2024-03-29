#ifndef __SRPC_RPC_SERVER_H__
#define __SRPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "fluent.hpp"
#include "vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "Protocol.h"
namespace srpc {

class RpcServer {
public:
    void ready();
    void run() { _server.run(); }
    void batch() { _server.batch(); }
    void stop() { _server.stop(); }
    fluent::Looper* looper() { return _server.looper(); }

    template <typename F>
    void bind(const std::string &method, const F &func);

    void onConnect(std::function<void(fluent::Context*)> callback);
    void onClose(std::function<void(fluent::Context*)> callback);
    void onRequest(std::function<bool(fluent::Context*, vsjson::Json&)> callback);
    void onResponse(std::function<void(fluent::Context*, vsjson::Json&)> callback);

    RpcServer(const fluent::InetAddress &address)
        : _server(address) {}
    RpcServer(const fluent::InetAddress &address,
              std::shared_ptr<fluent::Multiplexer> multiplexer)
        : _server(address, std::move(multiplexer)) {}
    ~RpcServer() = default;
    RpcServer(const RpcServer&) = delete;
    RpcServer(RpcServer&&) = default;
    RpcServer& operator=(const RpcServer&) = delete;
    RpcServer& operator=(RpcServer&&) = default;

private:
    vsjson::Json netCall(const std::string &method, vsjson::Json args);
    void reportError(vsjson::Json &response, const protocol::Exception &e);

private:
    std::map<std::string, std::function<vsjson::Json(vsjson::Json)>> _table;
    fluent::Server _server;
    Codec _codec;
    std::function<bool(fluent::Context*, vsjson::Json&)> _requestCallback;
    std::function<void(fluent::Context*, vsjson::Json&)> _responseCallback;
};

inline void RpcServer::ready() {
    _server.onMessage([this](fluent::Context *context) {
        auto &buffer = context->input;
        while(_codec.verify(buffer)) {
            auto request = _codec.decode(buffer);
            if(_requestCallback && !_requestCallback(context, request)) {
                continue;
            }
            auto id = request[protocol::Field::id].to<int>();
            auto &args = request[protocol::Field::params];
            auto method = std::move(request[protocol::Field::method]).to<std::string>();
            vsjson::Json response =
            {
                {protocol::Field::jsonrpc, protocol::Attribute::version},
                {protocol::Field::id, id},
            };

            try {
                response[protocol::Field::result] = netCall(method, std::move(args));
            } catch(const protocol::Exception &e) {
                reportError(response, e);
            } catch(const vsjson::JsonException &e) {
                reportError(response, protocol::Exception::makeParseErrorException());
            } catch(const std::exception &e) {
                reportError(response, protocol::Exception::makeInternalErrorException());
            }

            if(_responseCallback) {
                _responseCallback(context, response);
            }

            std::string dump = response.dump();
            uint32_t length = dump.length();
            uint32_t beLength = ::htonl(length);
            context->send(&beLength, sizeof(uint32_t));
            context->send(dump.c_str(), length);
        }
    });
    _server.ready();
}

template <typename F>
inline void RpcServer::bind(const std::string &method, const F &func) {
    _table[method] = CallProxy<F>(func);
}

inline void RpcServer::onConnect(std::function<void(fluent::Context*)> callback) {
    _server.onConnect(std::move(callback));
}

inline void RpcServer::onClose(std::function<void(fluent::Context*)> callback) {
    _server.onClose(std::move(callback));
}

inline void RpcServer::onRequest(std::function<bool(fluent::Context*, vsjson::Json&)> callback) {
    _requestCallback = std::move(callback);
}

inline void RpcServer::onResponse(std::function<void(fluent::Context*, vsjson::Json&)> callback) {
    _responseCallback = std::move(callback);
}

inline vsjson::Json RpcServer::netCall(const std::string &method, vsjson::Json args) {
    // assert args type == jsonArray
    auto methodHandle = _table.find(method);
    if(methodHandle != _table.end()) {
        return _table[method](std::move(args));
    } else {
        throw protocol::Exception::makeMethodNotFoundException();
    }
}

inline void RpcServer::reportError(vsjson::Json &response, const protocol::Exception &e) {
    if(response.contains(protocol::Field::result)) {
        auto &obj = response.as<vsjson::ObjectImpl>();
        obj.erase(protocol::Field::result);
    }
    response[protocol::Field::error] = {
        {protocol::Field::code, e.code()}
    };
    if(!e.message().empty()) {
        auto &err = response[protocol::Field::error];
        err[protocol::Field::message] = e.message();
    }
}

} // srpc
#endif