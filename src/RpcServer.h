#ifndef __SRPC_RPC_SERVER_H__
#define __SRPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "Protocol.h"
namespace srpc {

class RpcServer: private mutty::NonCopyable {
public:
    void start();

    template <typename F>
    void bind(const std::string &method, const F &func);

    vsjson::Json netCall(const std::string &method, vsjson::Json args);

    RpcServer(mutty::Looper *looper,
              const mutty::InetAddress &serverAddress)
        : _server(looper, serverAddress) {}
private:
    std::map<std::string, std::function<vsjson::Json(vsjson::Json)>> _table;
    mutty::Server _server;
    Codec _codec;
};

inline void RpcServer::start() {
    _server.onMessage([this](std::weak_ptr<mutty::TcpContext> context) {
        if(auto ctx = context.lock()) {
            auto &buffer = ctx->inputBuffer;
            while(_codec.verify(buffer)) {
                auto request = _codec.decode(buffer);
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
                    if(response.contains(protocol::Field::result)) {
                        auto &obj = response.as<vsjson::ObjectImpl>();
                        obj.erase(protocol::Field::result);
                        response[protocol::Field::error] = {
                            {protocol::Field::code, e.code}
                        };
                        if(!e.message.empty()) {
                            auto &err = response[protocol::Field::error];
                            err[protocol::Field::message] = std::move(e.message);
                        }
                    }
                }
                std::string dump = response.dump();
                uint32_t length = dump.length();
                uint32_t beLength = htonl(length);
                ctx->send(&beLength, sizeof(uint32_t));
                ctx->send(dump.c_str(), length);
            }
        }
    });
    _server.start();
}

template <typename F>
inline void RpcServer::bind(const std::string &method, const F &func) {
    _table[method] = CallProxy<F>(func);
}

inline vsjson::Json RpcServer::netCall(const std::string &method, vsjson::Json args) {
    // assert args type == jsonArray
    return _table[method](std::move(args));
}

} // srpc
#endif