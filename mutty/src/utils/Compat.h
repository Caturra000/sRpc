#ifndef __UTILS_COMPAT_H__
#define __UTILS_COMPAT_H__
#include <memory>

// 用于兼容部分缺失的特性
namespace cpp11 {

    // wont support T[]
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif