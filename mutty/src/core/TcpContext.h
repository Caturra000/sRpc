#ifndef __TCP_CONTEXT_H__
#define __TCP_CONTEXT_H__
#include <bits/stdc++.h>
#include "net/Socket.h"
#include "net/InetAddress.h"
#include "base/buffer/Buffer.h"
#include "base/buffer/BufferPool.h"
#include "base/timer/Timer.h"
#include "base/handler/Message.h"
#include "base/handler/MessageQueue.h"
#include "utils/Callable.h"
#include "utils/Pointer.h"
#include "base/context/ContextImpl.h"
namespace mutty {

class TcpHandler;
class TcpContext: public ContextImpl, public std::enable_shared_from_this<TcpContext> {
public:
    
// 状态机

    enum NetworkStatus { CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED };
    NetworkStatus networkStatus { CONNECTING };

// 网络特性支持

    Socket acceptedSocket;
    InetAddress localAddress, peerAddress;
    CachedBuffer inputBuffer, outputBuffer;

// 定时器特性支持

    Pointer<Timer> scheduler; // 如果需要用到定时器，只需添加这个即可


    // 适配器，绑定当前ctx，让一个functor(ctx)适配为Callable
    // 从Pointer换回raw ptr
    Callable binder(std::function<void(TcpContext*)> functor) {
        return Callable::make(std::move(functor), this);
    }

// wrapper

    bool isConnecting() { return networkStatus == CONNECTING; }
    bool isConnected() { return networkStatus == CONNECTED; }
    bool isDisConnecting() { return networkStatus == DISCONNECTING; }
    bool isDisConnected() { return networkStatus == DISCONNECTED; }
    void setConnecting() { networkStatus = CONNECTING; }
    void setConnected() { networkStatus = CONNECTED; }
    void setDisConnecting() { networkStatus = DISCONNECTING; }
    void setDisConnected() { networkStatus = DISCONNECTED; }

// in handle

    // 主动优雅关闭
    void shutdown(/*bool force = false*/) {
        if(/*force || */isConnected()) {
            setDisConnecting();
            if(_events & EVENT_WRITE) {
                acceptedSocket.shutdown();
            }
        }
        // 直到close才会DisConnected
    }


    // 主动handleClose
    void forceClose() {
        if(isConnected() || isDisConnecting()) {
            sendCloseMessage();
        }
    }

    void forceClose(Nanosecond delay) {
        std::weak_ptr<TcpContext> _this = shared_from_this();
        scheduler->runAfter(delay).with([_this] { 
            if(auto context = _this.lock()) {
                context->forceClose();
            }
        });
    }

    template <size_t N>
    void send(const char (&data)[N]) {
        send(data, N);
    }

    void send(const std::string &str) {
        send(str.c_str(), str.length());
    }

    // buffer存在时会由write event
    // TODO Direct模式
    void send(const void *data, int length) {
        if(!isDisConnected()) {
            // int wrote = 0;
            // if(!(_events & EVENT_WRITE) && outputBuffer.unread() == 0) {
            //     wrote = ::write(acceptedSocket.fd(), data, length);
            //     if(wrote > 0) {
            //         if(wrote == length) {
            //             // send complete
            //         }
            //     } // else may throw
            // }
            // if(wrote < length) {
            //     outputBuffer.append(static_cast<const char *>(data) + wrote, length - wrote);
            //     if(!(_events & EVENT_WRITE)) enableWrite();
            // }
            outputBuffer.append(static_cast<const char *>(data), length);
            if(!(_events & EVENT_WRITE)) enableWrite();
        }
    }

// ONLY FOR POLLER

    int fd() const override { return acceptedSocket.fd(); }


    TcpContext(Handler *handler, Looper *looper, 
            Socket acceptedSocket, InetAddress localAddress, InetAddress peerAddress)
        : ContextImpl(handler, looper),
          acceptedSocket(std::move(acceptedSocket)),
          localAddress(localAddress),
          peerAddress(peerAddress),
          scheduler(looper ? looper->getScheduler() : nullptr) {}

    // virtual ~TcpContext() { // Thread safe?
    //     disableRead();
    //     disableWrite();
    // }
};

} // mutty
#endif