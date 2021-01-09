#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "utils/Pointer.h"
namespace mutty {

class Handler;

// 表明一条TCP连接在发生某些事件时的消息
// 消息应该是轻量级的，可用值语义传递，且不持有任何生命周期
struct Message {
    Handler *target;
    int what;
    // EXTRA FIELD
    union {
        int flag; // 兼容原有方案
        int size;
    };
    void *any; // 搭配Defer传递
};

} // mutty
#endif