#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "utils/Pointer.h"

class Handler;

// 表明一条TCP连接在发生某些事件时的消息
// 消息应该是轻量级的，可用值语义传递，且不持有任何生命周期
struct Message {
    // Pointer<Handler> target; // 归属的handler，不存在可能delete的风险
    Handler *target; // 原先用Pointer只是想表明没有生命周期，且move是足够安全的，但是考虑到msg本身就是一次性的消费品，没这么麻烦，这些全部pod吧
    int what; // 表明一个什么样的事件，给handler看时可以通过switch case来确定具体的回调   
    // int flag;  // 由于_what和target有内存对齐的空洞，所以多设计一个arg，补充对_what的说明
    union {
        int flag; // 兼容原有方案
        int size; // 便于传递一些char*二进制流
    };
    void *any; // 考虑过使用std::any来保证类型安全和生命周期，但是觉得不必过度封装，必要时用Defer保证安全即可
    // 考虑大部分的message都没有跨handler处理的需求，用void*算是最简洁且不影响性能的方案了
};

#endif