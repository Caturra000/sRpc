#ifndef __TIMER_EVENT_H__
#define __TIMER_EVENT_H__
#include <bits/stdc++.h>
#include "utils/Callable.h"
#include "utils/Timestamp.h"
#include "utils/Algorithms.h"

struct TimerEvent {
    Timestamp _when;
    Nanosecond _interval;
    uint64_t _atMost;
    uint64_t _ticket;
    Callable _what;

    TimerEvent(Timestamp when, Callable what, Nanosecond interval = Millisecond::zero(), uint64_t atMost = 1)
        : _when(when), _what(std::move(what)), _interval(interval), _atMost(atMost) { }

    bool operator > (const TimerEvent &rhs) const {
        if(_when != rhs._when) return _when > rhs._when;
        return _ticket > rhs._ticket;
    }

    void operator() () const { _what.call(); }

    void next() {
        _when += _interval;
        _atMost--;
        if(!(_atMost & 7)) _ticket = random<uint64_t>();
    }
};

#endif