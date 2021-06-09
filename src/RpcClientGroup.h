#ifndef __SRPC_RPC_CLIENT_GROUP__
#define __SRPC_RPC_CLIENT_GROUP__
#include <bits/stdc++.h>
#include "mutty.hpp"
#include "RpcClient.h"
namespace srpc {

struct RpcClientGroupImpl {
    struct Comparator {
        bool operator()(const mutty::InetAddress &lhs,
                        const mutty::InetAddress &rhs) const;
    };
    using Container = std::multimap<mutty::InetAddress, RpcClient, Comparator>;
};

class RpcClientGroup: public RpcClientGroupImpl::Container {
public:
    using Base = RpcClientGroupImpl::Container;
    // using Base::Base;

    RpcClientGroup(mutty::Looper *looper)
        : Base(/*std::forward<Args>(args)...*/),
          _looper(looper) {}

    Base::iterator emplace(const mutty::InetAddress &serverAddress);

private:
    mutty::Looper *_looper;
};

inline bool RpcClientGroupImpl::Comparator
    ::operator()(const mutty::InetAddress &lhs,
                 const mutty::InetAddress &rhs) const {
    // InetAddress内存布局必然与sockaddr_in完全一致
    const auto &l = reinterpret_cast<const sockaddr_in&>(lhs);
    const auto &r = reinterpret_cast<const sockaddr_in&>(rhs);
    if(l.sin_addr.s_addr != r.sin_addr.s_addr) {
        return l.sin_addr.s_addr < r.sin_addr.s_addr;
    }
    return l.sin_port < r.sin_port;
}

inline RpcClientGroup::Base::iterator RpcClientGroup::emplace(const mutty::InetAddress &serverAddress) {
    return Base::emplace(std::piecewise_construct,
                         std::forward_as_tuple(serverAddress),
                         std::forward_as_tuple(_looper, serverAddress));
}

} // srpc
#endif
