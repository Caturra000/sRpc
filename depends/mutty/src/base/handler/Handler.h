#ifndef __HANDLER_H__
#define __HANDLER_H__
#include <bits/stdc++.h>
#include "utils/Callable.h"
#include "utils/TypeTraits.h"
#include "Message.h"
namespace mutty {

#define HANDLER_CALLBACK_DEFINE(functionName, callbackMember, ContextType, contextMember) \
    /* non-context */ \
    template <typename ...Args, typename = IsCallableType<Args...>> \
    void functionName(Args &&...args) { \
        callbackMember = Callable::make(std::forward<Args>(args)...); \
    } \
    /* simple context */ \
    template <typename Lambda, typename = IsCallableType<Lambda, ContextType*>> \
    void functionName(Lambda &&callback) { \
        callbackMember = Callable::make( \
            std::forward<Lambda>(callback), contextMember.get()); \
    } \
    /* safe for lifecycle */ \
    template <typename Lambda, typename U = IsCallableType<Lambda, std::weak_ptr<ContextType>>> \
    void functionName(Lambda &&callback, U* = nullptr) { \
        callbackMember = Callable::make( \
            std::forward<Lambda>(callback), contextMember); \
    }

class Handler {
public:
    // 处理消息的业务逻辑接口
    virtual void handle(Message msg) = 0;
    virtual ~Handler() { }
};

} // mutty
#endif