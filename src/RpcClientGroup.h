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

class RpcClientGroup: protected RpcClientGroupImpl::Container {
public:
    using Base = RpcClientGroupImpl::Container;
    // using Base::Base;

    // iterators
    using iterator = Base::iterator;
    using Base::begin;
    using Base::end;
    using Base::rbegin;
    using Base::rend;
    using Base::cbegin;
    using Base::cend;
    using Base::crbegin;
    using Base::crend;

    // capacity

    using Base::empty;
    using Base::size;
    using Base::max_size;

    // modifiers

    using Base::clear;
    // using Base::insert; // unsafe, use emplace
    using Base::erase;

    // lookup

    using Base::count;
    using Base::find;
    using Base::lower_bound;
    using Base::upper_bound;

    // wrapper

    auto emplace(const mutty::InetAddress &serverAddress) -> iterator;
    template <typename ...Args>
    auto emplace(Args &&...args) -> iterator;

    RpcClientGroup(mutty::Looper *looper);
    template <typename ...Forwards>
    RpcClientGroup(mutty::Looper *looper, Forwards &&...fs);

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

inline auto RpcClientGroup::emplace(const mutty::InetAddress &serverAddress) -> RpcClientGroup::iterator {
    return Base::emplace(std::piecewise_construct,
                         std::forward_as_tuple(serverAddress),
                         std::forward_as_tuple(_looper, serverAddress));
}

template <typename ...Args>
inline auto RpcClientGroup::emplace(Args &&...args) -> RpcClientGroup::iterator {
    // not a real in-place construction, but make interface clean
    return Base::emplace(std::piecewise_construct,
                         std::forward_as_tuple(std::forward<Args>(args)...),
                         std::forward_as_tuple(_looper, mutty::InetAddress(std::forward<Args>(args)...)));
}

inline RpcClientGroup::RpcClientGroup(mutty::Looper *looper)
    : Base(), _looper(looper) {}

template <typename ...Forwards>
inline RpcClientGroup::RpcClientGroup(mutty::Looper *looper, Forwards &&...fs)
    : Base(std::forward<Forwards>(fs)...),
      _looper(looper) {}

} // srpc
#endif
