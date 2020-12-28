#ifndef __UTILS_DEFER_H
#define __UTILS_DEFER_H
#include <bits/stdc++.h>
class Defer {
private:
    using DeferImpl = std::shared_ptr<Defer>;
    DeferImpl _defer;
public:
    template <typename T, typename ...Args>
    Defer(T &&callable, Args &&...args)
        : _defer(nullptr, [=](Defer*) { callable(std::move(args)...); }) { }
};
#endif