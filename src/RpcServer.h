#ifndef __SRPC_RPC_SERVER_H__
#define __SRPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "mutty.hpp"
#include "vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "RpcField.h"
namespace srpc {

class RpcServer: private mutty::NonCopyable {
public:
    void start();

    template <typename F>
    void bind(const std::string &name, const F &func);

    vsjson::Json netCall(const std::string &name, vsjson::Json args);

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
                auto token = request[RpcField::TOKEN].to<int>();
                auto &args = request[RpcField::ARGS];
                auto name = std::move(request[RpcField::NAME]).to<std::string>();
                vsjson::Json response =
                {
                    {RpcField::TOKEN, token},
                };

                response[RpcField::RETURN] = netCall(name, std::move(args));

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
inline void RpcServer::bind(const std::string &name, const F &func) {
    _table[name] = CallProxy<F>(func);
}

inline vsjson::Json RpcServer::netCall(const std::string &name, vsjson::Json args) {
    // assert args type == jsonArray
    return _table[name](std::move(args));
}

} // srpc
#endif