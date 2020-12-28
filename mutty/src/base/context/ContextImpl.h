#ifndef __CONTEXT_IMPL_H__
#define __CONTEXT_IMPL_H__
#include <poll.h>
#include <bits/stdc++.h>
#include "Context.h"
#include "base/handler/MessageQueue.h"
#include "base/handler/Message.h"
#include "utils/Pointer.h"
#include "core/Looper.h"
#include "core/Multiplexer.h"

class Handler;

// 提供基本的poller接口支持以及handler机制默认实现
// 具体的Context实现类可以直接继承，并给出fd
class ContextImpl: public Context {
public:
    void sendMessage(int what) { _messageQueue->post({_handler.get(), what}); } 
    void sendMessage(int what, int flag) { _messageQueue->post({_handler.get(), what, flag}); }

    // 接口需要手动控制data生命周期，一般搭配Defer食用
    void sendMessageWithData(int what, void *data) { 
        _messageQueue->post( Message {
            .target = _handler.get(),
            .what = what,
            .any = data
        });    
    }
    
    void sendMessageWithBinaryData(int what, int size, void *data) {
        _messageQueue->post({_handler.get(), what, size, data});
    }

    void sendReadMessage()  override { sendMessage(MSG_POLL_READ);  }
    void sendWriteMessage() override { sendMessage(MSG_POLL_WRITE); }
    void sendErrorMessage() override { sendMessage(MSG_POLL_ERROR); }
    void sendCloseMessage() override { sendMessage(MSG_POLL_CLOSE); }


    uint32_t events() const override { return _events; }
    void updateState() override {
        if(_state == STATE_NEW || _state == STATE_DELETED) {
            _state = STATE_ADDED;
            _multiplexer->update(EPOLL_CTL_ADD, this);
        } else { // STATE_ADDED
            if(_events == EVENT_NONE) {
                _multiplexer->update(EPOLL_CTL_DEL, this);
                _state = STATE_DELETED;
            } else {
                _multiplexer->update(EPOLL_CTL_MOD, this);
            }
        }
    }

    void enableRead()   override {
        if(!(_events & EVENT_READ)) {
            _events |= EVENT_READ;
            updateState(); 
        }
    }
    void enableWrite()  override {
        if(!(_events & EVENT_WRITE)) {
            _events |= EVENT_WRITE;
            updateState(); 
        }
    }
    void disableRead()  override {
        if(_events & EVENT_READ) {
            _events &= ~EVENT_READ;
            updateState();
        }
    }
    void disableWrite() override {
        if(_events & EVENT_WRITE) {
            _events &= ~EVENT_WRITE;
            updateState();
        }
    }
    bool readEnabled()  override { return _events & EVENT_READ; }
    bool writeEnabled() override { return _events & EVENT_WRITE; }
    // error / close

    ContextImpl(Handler *handler = nullptr, Looper *looper = nullptr)
        : _handler(handler),
          _messageQueue(looper ? looper->getProvider() : nullptr),
          _multiplexer(looper ? looper->getPoller() : nullptr),
          _state(STATE_NEW), _events(EVENT_NONE) { }

    // fd()

protected:
    Pointer<Handler> _handler;
    Pointer<MessageQueue> _messageQueue;
    Pointer<Multiplexer> _multiplexer;

    int _state;
    uint32_t _events;

// for state

    constexpr static int STATE_NEW = 0;
    constexpr static int STATE_ADDED = 1;
    constexpr static int STATE_DELETED = 2;

// for events

    constexpr static uint32_t EVENT_NONE = 0;
    constexpr static uint32_t EVENT_READ = POLL_IN | POLL_PRI;
    constexpr static uint32_t EVENT_WRITE = POLL_OUT;
    
    
};

#endif
