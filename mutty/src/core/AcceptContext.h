#ifndef __ACCEPT_CONTEXT_H
#define __ACCEPT_CONTEXT_H
#include <fcntl.h>
#include "utils/Pointer.h"
#include "utils/Exchanger.h"
#include "base/context/ContextImpl.h"
#include "net/Socket.h"
#include "net/InetAddress.h"
class Handler;
class AcceptContext: public ContextImpl {
public:

    CONTEXT_MSG_DEFINE(MSG_SOCKET_LISTEN);
    CONTEXT_MSG_DEFINE(MSG_ACCEPT_WITH_DATA);

    int fd() const override { return acceptSocket.fd(); }

// 上下文相关

    Socket acceptSocket;
    InetAddress localAddress;

// 跨组件交互特性支持

    Exchanger exchanger;


    AcceptContext(Handler *handler, Looper *looper, InetAddress localAddress)
        : ContextImpl(handler, looper),
          localAddress(localAddress) {
        using Option = Socket::Option;
        acceptSocket.config(Option::REUSE_PORT | Option::REUSE_ADDR);
        acceptSocket.bind(localAddress);
    }
};
#endif