#ifndef __LOOPER_POOL_H__
#define __LOOPER_POOL_H__
#include <bits/stdc++.h>
#include "utils/Algorithms.h"
#include "utils/ThreadPool.h"
#include "utils/Compat.h"
#include "Looper.h"

// 具体用途是每次accept都分配到固定大小的Looper池中
template <size_t N>
class LooperPool {
public:
    std::unique_ptr<Looper>& pick() {
        auto index = random<size_t>() & N-1;
        return _pool[index]; // ready
    }

    LooperPool(): _threads(N) {
        constexpr size_t mayBeZero = N & N-1;
        static_assert(mayBeZero == 0, "N must be power of two.");
        for(auto &looper : _pool) {
            looper = cpp11::make_unique<Looper>();
            _threads.execute([&] {looper->loop();});
        }
    }
    
    // TODO 析构问题


private:
    std::array<std::unique_ptr<Looper>, N> _pool;
    ThreadPool _threads;
};

#endif