#ifndef __TCP_POLICY_H__
#define __TCP_POLICY_H__
#include <bits/stdc++.h>
#include "core/TcpHandler.h"


#define TCP_POLICY_CALLBACK_DEFINE(callback, policy) \
    template <typename ...Args, typename = IsCallableType<Args...>> \
    void callback(Args &&...args) { \
        policy = cpp11::make_unique<TcpPolicyImpl<Callable>>( \
            Callable::make(std::forward<Args>(args)...)); \
    } \
    template <typename Lambda> \
    void callback(Lambda &&functor) { \
        policy = cpp11::make_unique<TcpPolicyImpl<Lambda>>(std::forward<Lambda>(functor)); \
    } 



// 用于泛型擦除
struct TcpPolicy {
    //virtual void setHandler(TcpHandler*) = 0;
    virtual void onConnect(TcpHandler*) = 0;
    virtual void onMessage(TcpHandler*) = 0;
    virtual void onWriteComplete(TcpHandler*) = 0;
    virtual void onClose(TcpHandler*) = 0;
    virtual ~TcpPolicy() { }
};


template <typename T>
struct TcpPolicyImpl: public TcpPolicy {
    typename std::decay<T>::type runtimeInfo;
    TcpPolicyImpl(T info): runtimeInfo(std::move(info)) {}
    void onConnect(TcpHandler *connection) override
        { connection->onConnect(runtimeInfo);} // copy
    void onMessage(TcpHandler *connection) override
        { connection->onMessage(runtimeInfo);}
    void onWriteComplete(TcpHandler *connection) override
        { connection->onWriteComplete(runtimeInfo);}
    void onClose(TcpHandler *connection) override
        { connection->onClose(runtimeInfo);} 
};

#endif