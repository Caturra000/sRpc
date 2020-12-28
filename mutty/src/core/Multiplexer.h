#ifndef __MULTIPLEXER_H__
#define __MULTIPLEXER_H__
#include <bits/stdc++.h>
#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>
#include "utils/Timestamp.h"
#include "base/context/Context.h"
#include "throws/Exceptions.h"

// 负责epoll的封装
class Multiplexer {
private:
    std::vector<epoll_event> _events;
    int _epollFd;

public:
    Multiplexer(): _epollFd(epoll_create1(EPOLL_CLOEXEC)), _events(16) {
        if(_epollFd < 0) throw EpollCreateException(errno);
    }
    ~Multiplexer() { close(_epollFd); }

    void poll(Nanosecond timeout) { //支持ns的表示，虽然实现上用不着，因为epoll粒度是ms
        int count = epoll_wait(_epollFd, _events.data(), _events.size(), 
                        std::chrono::duration_cast<Millisecond>(timeout).count());
        if(count < 0) throw EpollWaitException(errno);
        dispatchActiveContext(count);
        if(_events.size() == count) {
            _events.reserve(_events.size() << 1);
        }
    }

    void update(int operation, Context *ctx) {
        epoll_event event {ctx->events(), ctx};
        if(epoll_ctl(_epollFd, operation, ctx->fd(), &event)) {
            throw EpollControlException(errno);
        }
            // TODO
        // }
    }

private:
    void dispatchActiveContext(int count) {
        for(int i = 0; i < count; ++i) {
            auto ctx = static_cast<Context*>(_events[i].data.ptr);
            auto revent = _events[i].events;
            dispatch(ctx, revent);
        }
    }

    // 一个简单粗暴的实现
    void dispatch(Context *ctx, int revent) {
        if((revent & POLLHUP) && !(revent & POLLIN)) {
            ctx->sendCloseMessage();
        }
        if(revent & (POLLERR | POLLNVAL)) {
            if(!errno) ctx->sendCloseMessage(); // RST?
            else ctx->sendErrorMessage();
        }
        if(revent & (POLLIN | POLLPRI | POLLRDHUP)) {
            ctx->sendReadMessage();
        }
        if(revent & POLLOUT) {
            ctx->sendWriteMessage();
        }
    }
};

#endif