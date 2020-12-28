#ifndef __ACCEPT_HANDLER_H__
#define __ACCEPT_HANDLER_H__
#include <bits/stdc++.h>
#include "utils/Pointer.h"
#include "base/handler/MessageQueue.h"
#include "base/handler/Handler.h"
#include "net/Socket.h"
#include "AcceptContext.h"

class AcceptHandler: public Handler {
public:

    void handle(Message msg) override {
        switch(msg.what) {
            case AcceptContext::MSG_POLL_READ:
                handleRead(msg.flag);
            break;
            case AcceptContext::MSG_SOCKET_LISTEN:
                // handleListen();
            break;
            default:
            ;
        }
    }

// handle事件接口

    // UNUSED who
    void handleRead(int who = 0) {
        InetAddress peerAddress;
        Socket connectSocket = _ctx->acceptSocket.accept(peerAddress);
        _ctx->exchanger = std::pair<Socket, InetAddress>(
            std::move(connectSocket), std::move(peerAddress));
        _newConnectionCallback(); // add to ConnectionPool of Server
    }

// callback定义

    // onNewConnection注册时，context的exchanger携带连接信息，如果不处理将会某个时刻自行析构
    HANDLER_CALLBACK_DEFINE(onNewConnection, _newConnectionCallback, AcceptContext, _ctx)


    AcceptHandler(Looper *looper, InetAddress localAddress)
        : _ctx(std::make_shared<AcceptContext>(this, looper, localAddress)) {}


    void init() {
        _ctx->acceptSocket.listen();
        _ctx->enableRead();
    }

protected:
    std::shared_ptr<AcceptContext> _ctx;

    Callable _newConnectionCallback;
};
#endif