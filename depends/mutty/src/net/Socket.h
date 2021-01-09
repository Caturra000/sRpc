#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utils/Noncopyable.h"
#include "InetAddress.h"
#include "netinet/tcp.h"
#include "throws/Exceptions.h"
namespace mutty {


// 针对socket fd的浅层封装，要求只有fd一个成员
class Socket: public Noncopyable {
public:
    static constexpr int INVALID_FD = -1;
    Socket(): _socketFd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) 
        { if(_socketFd < 0) throw SocketCreateException(errno); }
    ~Socket() { if(_socketFd != INVALID_FD) ::close(_socketFd); }
    explicit Socket(int socketFd): _socketFd(socketFd) { assert(_socketFd != INVALID_FD); } // unsafe
    Socket(Socket &&rhs): _socketFd(rhs._socketFd) { rhs._socketFd = INVALID_FD; }
    Socket& operator=(Socket &&rhs) {
        if(this == &rhs) return *this;
        _socketFd = rhs._socketFd;
        rhs._socketFd = INVALID_FD;
        return *this;
    }

    // int fd = std::move(socket);
    operator int() && {
        int fd = _socketFd;
        _socketFd = INVALID_FD;
        return fd;
    }
    
    // TODO remove
    struct Option {
        const static int NO_DELAY   = 1 << 0;
        const static int REUSE_ADDR = 1 << 1;
        const static int REUSE_PORT = 1 << 2;
        const static int KEEP_ALIVE = 1 << 3;

        const static int DEFAULT      = 0;
        const static int FULL_FEATURE = std::numeric_limits<int>::max(); // (1LL << 31) - 1; 

        const int _option;

        Option(int option = DEFAULT): _option(option) {}
    };

    int fd() const { return _socketFd; }

    void config(Option option);


public:

// wrapper

    void bind(const InetAddress &address);
    void listen(int backlog = SOMAXCONN);
    void bindAndListen(const InetAddress &serverAddress, int backlog = SOMAXCONN) 
        { bind(serverAddress); listen(backlog); }
    Socket accept(InetAddress &clientAddress);
    Socket accept();
    int connect(const InetAddress &address);
    void detach() { _socketFd = INVALID_FD; }
    void shutdown() { ::shutdown(_socketFd, SHUT_WR); }

// setter

    void setNoDelay(bool on = true);
    void setReuseAddr(bool on = true);
    void setReusePort(bool on = true);
    void setKeepAlive(bool on = true);

    // 更多的tcp/socket选项还是给unix api来做吧

// for client usage

    void setBlock();
    void setNonBlock();

private:
    int _socketFd;
};

inline void Socket::bind(const InetAddress &address) { 
    if(::bind(_socketFd, (const sockaddr*)(&address), sizeof(InetAddress))) {
        throw SocketBindException(errno);
    } 
}
inline void Socket::listen(int backlog) { 
    if(::listen(_socketFd, backlog)) {
        throw SocketListenException(errno); 
    }
}


inline Socket Socket::accept(InetAddress &address) {
    socklen_t len = sizeof(address);
    int connectFd = ::accept4(_socketFd, (sockaddr*)(&address), &len,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connectFd < 0) throw SocketAcceptException(errno);
    return Socket(connectFd);
}

inline Socket Socket::accept() {
    int connectFd = ::accept4(_socketFd, nullptr, nullptr,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
    return Socket(connectFd);
}

inline int Socket::connect(const InetAddress &address) {
    // 不抛出异常
    return ::connect(_socketFd, (const sockaddr *)(&address), sizeof(address));
}

inline void Socket::config(Option option) {
    int opt = option._option;
    if(opt & Option::NO_DELAY) setNoDelay();
    if(opt & Option::REUSE_ADDR) setReuseAddr();
    if(opt & Option::REUSE_PORT) setReusePort();
    if(opt & Option::KEEP_ALIVE) setKeepAlive();
}

inline void Socket::setNoDelay(bool on) {
    int optval = on;
    if(setsockopt(_socketFd, IPPROTO_TCP, TCP_NODELAY, &optval,
            static_cast<socklen_t>(sizeof optval))) {
        throw SocketException(errno);
    }
}

inline void Socket::setReuseAddr(bool on) {
    int optval = on;
    if(setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &optval,
            static_cast<socklen_t>(sizeof optval))) {
        throw SocketException(errno);
    }
}

inline void Socket::setReusePort(bool on) {
    int optval = on;
    if(setsockopt(_socketFd, SOL_SOCKET, SO_REUSEPORT, &optval,
            static_cast<socklen_t>(sizeof optval))) {
        throw SocketException(errno);
    }
}

inline void Socket::setKeepAlive(bool on) {
    int optval = on;
    if(setsockopt(_socketFd, SOL_SOCKET, SO_KEEPALIVE, &optval,
            static_cast<socklen_t>(sizeof optval))) {
        throw SocketException(errno);
    }
}


inline void Socket::setBlock() {
    int flags = ::fcntl(_socketFd, F_GETFL, 0);
    ::fcntl(_socketFd, F_SETFL, flags & (~SOCK_NONBLOCK));
}

inline void Socket::setNonBlock() {
    int flags = ::fcntl(_socketFd, F_GETFL, 0);
    ::fcntl(_socketFd, F_SETFL, flags | SOCK_NONBLOCK);
}

} // mutty

#endif