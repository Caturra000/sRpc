#ifndef __RPC_RPC_SERVER_H__
#define __RPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
#include "RpcField.h"
using namespace mutty;
using namespace vsjson;


class RpcServer {
public:
    RpcServer(Looper *looper, const InetAddress &serverAddress)
        : _server(looper, serverAddress) {
    }

    void start() {
        _server.onConnect([] {
            std::cout << "connected" << std::endl;
        });
        _server.onMessage([this](std::weak_ptr<TcpContext> context) {
            if(auto ctx = context.lock()) {
                auto &buffer = ctx->inputBuffer;
                if(_codec.verify(buffer)) {
                    auto request = _codec.decode(buffer);
                    auto token = request[RpcField::TOKEN].to<int>();
                    auto &args = request[RpcField::ARGS];
                    auto name = std::move(request[RpcField::NAME]).to<std::string>();
                    Json response =
                    {
                        {RpcField::TOKEN, token},
                    };

                    response[RpcField::RETURN] = netCall(name, std::move(args));

                    std::string dump = response.dump();
                    uint32_t length = dump.length();
                    uint32_t beLength = htonl(length);
                    ctx->send(&beLength, sizeof(uint32_t));
                    ctx->send(dump.c_str(), length);

                    std::cout << "sent!" << std::endl;
                    std::cout << "{" << length << dump << "}" << std::endl;
                }
            }
        });
        _server.start(); 
    }
    template <typename F>
    void bind(const std::string &name, const F &func) {
        _table[name] = CallProxy<F>(func);
    }


    // args是JsonArray类型
    Json netCall(const std::string &name, Json args) {
        return _table[name](std::move(args));
    }
private:
    std::map<std::string, std::function<Json(Json)>> _table;
    Server _server;
    Codec _codec;
};
#endif