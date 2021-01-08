#ifndef __TIMER_H__
#define __TIMER_H__
#include <bits/stdc++.h>
#include "TimerEvent.h"
#include "utils/Timestamp.h"
#include "utils/Defer.h"
#include "utils/Algorithms.h"
#include "utils/Pointer.h"
namespace mutty {

class Timer {
public:
    using EventHeap = std::priority_queue<
            TimerEvent, std::vector<TimerEvent>, std::greater<TimerEvent>>;
    using ResultSet = std::vector<TimerEvent>;
    Timer() = default;
    Timer(std::initializer_list<TimerEvent> eventList): Timer() {
        for(auto &e : eventList) {
            _container.push(std::move(const_cast<TimerEvent&>(e))); //FIXME:可能需要改接口为append
        }
    }

    template <typename ...EventConstructorArgs>
    Timer& append(EventConstructorArgs &&...args) {
        std::lock_guard<std::mutex> _{_mutex};
        _container.push(TimerEvent(std::forward<EventConstructorArgs>(args)...));
        return *this;
    }

    // 不会阻塞，只会执行到不符合条件的时间就返回
    Millisecond run() {
        std::lock_guard<std::mutex> _ {_mutex};
        if(_container.empty()) return Millisecond::zero();
        Timestamp current = now();
        while(!_container.empty()) {
            auto &e = _container.top();
            if(e._when > current) break;
            Defer _ {[this] { _container.pop(); }};
            if(e._atMost > 0) e._what.call();
            if(e._atMost > 1) {
                _reenterables.push_back(std::move(const_cast<TimerEvent&>(e)));
                auto &b = _reenterables.back();
                b.next();
            }
        }
        for(auto &e : _reenterables) _container.push(std::move(e)); // 同event在单次run只运行一次
        _reenterables.clear();
        if(_container.empty()) return Millisecond::zero();
        return std::chrono::duration_cast<Millisecond>(_container.top()._when - now());
    }

    // 避免call在timer中调用，减少可能的锁争用
    // TODO 目前没有保证call()在时间上的严格升序，作为scheduler可能不安全（单次运行是可保证的）
    Millisecond run(ResultSet &tasks) {
        std::lock_guard<std::mutex> _ {_mutex};
        Timestamp current = now();
        for(size_t n = tasks.size(); n; --n) {
            auto &task = tasks.back();
            if(task._atMost > 1) {
                task.next();
                // TODO 使用std::list维护reenter时的ticket
                //      当优先级较高时插入头部，否则插入尾部
                if(task._when > current) {
                    _container.emplace(std::move(task));
                } else {
                    _reenterables.emplace_back(std::move(task));
                }
            }
            tasks.pop_back();
        }
        _reenterables.swap(tasks); // next time
        if(_container.empty()) return Millisecond::zero();
        while(!_container.empty()) {
            auto &task = _container.top();
            if(task._when > current) break;
            tasks.emplace_back(std::move(std::move(const_cast<TimerEvent&>(task))));
            _container.pop();
        }
        // TODO 记录tasks中atMost > 1的下一次when的min
        if(_container.empty()) return Millisecond::zero();
        return std::chrono::duration_cast<Millisecond>(_container.top()._when - now());
    }

// Builder Start

    enum Urgency {
        MINOR = std::numeric_limits<uint64_t>::max(),
        MAJOR = MINOR >> 30,
        CRITICAL = MAJOR >> 30,
    };

    struct TimerHelper {
        Timestamp _when;
        Nanosecond _interval;
        uint64_t _atMost;
        Timer *_thisTimer;

        template <typename ...Args>
        TimerHelper& with(Args &&...args) { 
            _thisTimer->append(_when, 
                Callable::make(std::forward<Args>(args)...), 
                _interval, _atMost); 
            return *this;
        }

        TimerHelper& per(Nanosecond interval) {
            _interval = interval;
            return *this;
        }
        TimerHelper& atMost(uint64_t times) {
            _atMost = times;
            return *this;
        }
        TimerHelper at(Timestamp when) {
            _when = when;
            return *this;
        }

        TimerHelper priority(Urgency urgency) {
            _atMost = urgency;
            return *this;
        }
    };

    // usage:
    // runAfter(2s).per(2s).atMost(3).with(func, arg0, arg1);
    // runEvery(500ms).at(now()+1s).with([]{std::cerr<<"b";});

    TimerHelper runAt(Timestamp when) 
        { return TimerHelper{when, Millisecond::zero(), 1, this}; }
    TimerHelper runAfter(Nanosecond interval) 
        { return TimerHelper{nowAfter(interval), Millisecond::zero(), 1, this}; }
    TimerHelper runEvery(Nanosecond interval) 
        { return TimerHelper{now(), interval, std::numeric_limits<uint64_t>::max(), this}; }

// Builder End

private:
    EventHeap _container;
    bool _running {false};
    std::mutex _mutex;
    ResultSet _reenterables;
};

} // mutty
#endif