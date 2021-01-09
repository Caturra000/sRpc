#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__
#include <bits/stdc++.h>
#include "Message.h"
namespace mutty {

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

} // mutty
#endif