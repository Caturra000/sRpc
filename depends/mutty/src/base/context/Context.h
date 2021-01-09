#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <bits/stdc++.h>
namespace mutty {

// 命名规范： MSG_<Message>
#define CONTEXT_MSG_DEFINE(messageType) \
    const static int messageType = __COUNTER__+1 

// 只提供接口部分，该接口会应用于IO复用
class Context {
public:

// message define

    CONTEXT_MSG_DEFINE(MSG_POLL_READ);
    CONTEXT_MSG_DEFINE(MSG_POLL_WRITE);
    CONTEXT_MSG_DEFINE(MSG_POLL_ERROR);
    CONTEXT_MSG_DEFINE(MSG_POLL_CLOSE);

// for handler

    virtual void sendReadMessage() {}
    virtual void sendWriteMessage() {}
    virtual void sendErrorMessage() {}
    virtual void sendCloseMessage() {}

// for multiplexer

    virtual int fd() const = 0;
    virtual uint32_t events() const = 0;
    virtual void updateState() = 0;

// self permission

    virtual void enableRead() {}
    virtual void enableWrite() {}
    virtual void disableRead() {}
    virtual void disableWrite() {}
    virtual bool readEnabled() { return false; }
    virtual bool writeEnabled() { return false;}
    // add remove change

    virtual ~Context() {}
};

// using ContextImpl = Context;

} // mutty

#endif