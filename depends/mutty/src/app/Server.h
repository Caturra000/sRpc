#ifndef __SERVER_H__
#define __SERVER_H__
#include <bits/stdc++.h>
#include "utils/Pointer.h"
#include "utils/Exchanger.h"
#include "utils/Object.h"
#include "utils/TypeTraits.h"
#include "utils/Compat.h"
#include "utils/FastIo.h"
#include "base/handler/MessageQueue.h"
#include "core/Looper.h"
#include "core/AcceptHandler.h"
#include "core/ConnectionPool.h"
#include "core/TcpHandler.h"
#include "core/TcpPolicy.h"
namespace mutty {

class Server {
public:
    Server(Looper *looper, InetAddress localAddress)
        : _looper(looper),
          _acceptor(looper, localAddress) {
       
    }

    // 二阶段构造，在start()前应处理好tcpHandler的回调注册
    void start() {
         _acceptor.onNewConnection([this](std::weak_ptr<AcceptContext> context) {
            if(auto ctx = context.lock()) {
                auto connectionInfo = std::move(cast<
                    std::pair<Socket, InetAddress>&>(ctx->exchanger));
                Socket &connectionSocket = connectionInfo.first;
                InetAddress &peerAddress = connectionInfo.second;
                auto &connection = _connections.createNewConnection(
                    std::move(connectionSocket), ctx->localAddress, peerAddress);
                tcpCallbackInit(connection.get());
                connection->init();
            }
        });
        _acceptor.init();
    }


private:
    void tcpCallbackInit(TcpHandler*);


// for TcpHandler

    // 使用unique_ptr提供RAII和多态

    std::unique_ptr<TcpPolicy> _connectPolicy;
    std::unique_ptr<TcpPolicy> _messagePolicy;
    std::unique_ptr<TcpPolicy> _writeCompletePolicy;
    std::unique_ptr<TcpPolicy> _closePolicy;

public:

// server回调注册接口

    TCP_POLICY_CALLBACK_DEFINE(onConnect, _connectPolicy)
    TCP_POLICY_CALLBACK_DEFINE(onMessage, _messagePolicy)
    TCP_POLICY_CALLBACK_DEFINE(onWriteComplete, _writeCompletePolicy)
    TCP_POLICY_CALLBACK_DEFINE(onClose, _closePolicy)

private:
    Pointer<Looper> _looper;
    AcceptHandler _acceptor;
    ConnectionPool _connections; 
    // LooperPool与IoLooper同一生命周期，保证了Handler与context在回调时的生命周期
};

inline void Server::tcpCallbackInit(TcpHandler *connection) {
    if(_connectPolicy) _connectPolicy->onConnect(connection);
    if(_messagePolicy) _messagePolicy->onMessage(connection);
    if(_writeCompletePolicy) _writeCompletePolicy->onWriteComplete(connection);
    if(_closePolicy) _closePolicy->onClose(connection);
}

} // mutty
#endif