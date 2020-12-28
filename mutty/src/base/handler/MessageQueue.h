#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__
#include <bits/stdc++.h>
#include "Message.h"

// 一个用于MPSC场合的消息队列
// 如果确定msg都是从同一个Multiplexer传过来的，那么是可以保证无需上锁
// 但是用到外围的特性（目前只有定时器）则需要上锁，这里没用到timerfd，而是额外的线程去实现定时
class MessageQueue {
public:
    int size() const { return _queue.size(); }
    bool hasNext() const { return !_queue.empty(); }
    Message next(); // 不保证线程安全，由Looper控制
    void post(Message msg);
    MessageQueue() = default;
    ~MessageQueue() = default;
    // 需要定制move行为，不移动mutex
    MessageQueue(MessageQueue &&rhs): _queue(std::move(rhs._queue)) {}
    MessageQueue& operator=(MessageQueue &&rhs) {
        if(this == &rhs) return *this;
        _queue = std::move(rhs._queue);
        return *this;
    }

    // used only for @Looper
    std::mutex& lock() { return _mutex; }
    bool hasNextUnlock() {
        std::lock_guard<std::mutex> _{_mutex};
        return !_queue.empty();
    }

private:
    std::queue<Message> _queue;
    std::mutex _mutex;
};

inline void MessageQueue::post(Message msg) {
    std::lock_guard<std::mutex> _{_mutex};
    _queue.push(msg); 
}

inline Message MessageQueue::next() {
    if(_queue.empty()) return {};
    auto msg = _queue.front();
    _queue.pop();
    return msg;
}
#endif