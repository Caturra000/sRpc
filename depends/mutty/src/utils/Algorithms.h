#ifndef __UTILS_ALGORITHMS_H__
#define __UTILS_ALGORITHMS_H__
namespace mutty {

auto roundToPowerOfTwo = [](int v)->int {
    v--;
    v |= v >> 1; v |= v >> 2;
    v |= v >> 4; v |= v >> 8;
    v |= v >> 16;
    return ++v;
};

auto lowbit = [](int v) { return v&-v; };
auto isPowerOfTwo = [](int v) { return v == lowbit(v); };
auto highestBitPosition = [](int v) { 
    for(int i = 31; ~i; --i) {
        if(v>>i&1) return i;
    }
    return 0;
};

auto ceilOfPowerOfTwo = [](int v) {
    int n = highestBitPosition(v);
    if(isPowerOfTwo(v)) return n;
    return n+1;
};

auto floorOfPowerOfTwo = [](int v) {
    int n = highestBitPosition(v);
    if(isPowerOfTwo(v)) return n;
    return n-1;
};

// 实现一个线程安全的简易随机数生成器
template <typename T = unsigned long long, unsigned long long init = 19260817>
inline T random() {
    static thread_local auto seed = init;
    seed = seed * 998244353 + 12345;
    return static_cast<T>(seed / 1024);
}

} // mutty
#endif