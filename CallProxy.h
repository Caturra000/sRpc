#ifndef __RPC_CALL_PROXY_H__
#define __RPC_CALL_PROXY_H__
#include <bits/stdc++.h>
#include "vsjson/vsjson.hpp"
using namespace vsjson;

template <typename F>
class CallProxy {
public:
    
    CallProxy(F func): _func(std::move(func)) {}
    Json operator()(const char* stream, size_t len) {
        // 隐式转换
        return dispatch(_func, stream, len); // FIXME 返回引用会decay
    }

private:

    // 函数指针
    template <typename Ret, typename ...Args, 
    typename WrappedRet = std::conditional_t<
        std::is_same<Ret, void>::value, nullptr_t, Ret>>
    WrappedRet dispatch(Ret (*func)(Args...), const char *&stream, size_t len) {
        using ArgsTuple = std::tuple<std::decay_t<Args>...>;
        constexpr size_t N = sizeof...(Args);
        // 从stream构造tuple
        ArgsTuple argsTuple = make<ArgsTuple>(stream, std::make_index_sequence<N>{});
        // 用tuple构造回原来的args，调用并返回
        WrappedRet result = invoke<Ret>(argsTuple); 
        return result; // 如果为void，返回nullptr
    }

    // std::function
    template <typename Ret, typename ...Args, 
    typename WrappedRet = std::conditional_t<
        std::is_same<Ret, void>::value, nullptr_t, Ret>>
    WrappedRet dispatch(std::function<Ret(Args...)> &func, const char *&stream, size_t len) {
        using ArgsTuple = std::tuple<std::decay_t<Args>...>;
        constexpr size_t N = sizeof...(Args);
        // 从stream构造tuple
        ArgsTuple argsTuple = make<ArgsTuple>(stream, std::make_index_sequence<N>{});
        // 用tuple构造回原来的args，调用并返回
        WrappedRet result = invoke<Ret>(argsTuple); 
        return result; // 如果为void，返回nullptr
    }

    // 成员函数  FIXME 接口不一致
    // template <typename Ret, typename Class, typename ...Args, 
    // typename WrappedRet = std::conditional_t<
    //     std::is_same<Ret, void>::value, nullptr_t, Ret>>
    // WrappedRet dispatch(Ret (Class::*func)(Args...), Class *obj, 
    //                     const char *stream, size_t len) {
    //     using ArgsTuple = std::tuple<std::decay_t<Args>...>;
    //     constexpr size_t N = sizeof...(Args);
    //     ArgsTuple argsTuple = make<ArgsTuple>(stream, std::make_index_sequence<N>{});
    //     auto wrappedFunc = [=](Args... args) -> Ret { return (obj->*func)(args...); };
    //     WrappedRet result = invoke<Ret>(argsTuple); 
    //     return result;
    // }

    // TODO  lambda

    // 从stream中处理整个tuple
    template <typename Tuple, size_t ...Is>
    Tuple make(const char *&from, std::index_sequence<Is...>) {
        Tuple tuple;
        std::initializer_list<int> { (get<Tuple, Is>(from, tuple), 0)... };
        return tuple;
    }

    // TODO const char* / char* 需要额外处理size和生命周期
    // 或者不接受任何指针形式，只处理定长struct， 后续加强到仅限POD
    // 从stream中处理单个elem
    template <typename Tuple, size_t I, typename ElemType = std::decay_t<decltype(std::get<I>(Tuple{}))>>
    auto get(const char *&from, Tuple &to) -> std::enable_if_t<!std::is_pointer<ElemType>::value> {
        constexpr size_t size = sizeof(std::get<I>(to));
        memcpy(std::addressof(std::get<I>(to)), from, size);
        from += size;
    }

    // 指针不好处理
    // template <typename Tuple, size_t I, typename ElemType = std::decay_t<decltype(std::get<I>(Tuple{}))>>
    // auto get(const char *&from, Tuple &to) -> std::enable_if_t<std::is_pointer<ElemType>::value> {
    //     new(std::addressof(std::get<I>(to))) ElemType((ElemType)from);  // FIXME
    //     from += strlen(from) + 1; // +'\0'
    // }
    
    // 通过tuple的形式调用func
    template <typename Ret, typename Tuple,
    typename = std::enable_if_t<!std::is_same<Ret, void>::value>>
    Ret invoke(Tuple &&tuple) {
        constexpr size_t N = std::tuple_size<std::decay_t<Tuple>>::value;
        return invokeImpl<Ret>(std::forward<Tuple>(tuple), std::make_index_sequence<N>{});
    }

    template <typename Ret, typename Tuple,
    typename = std::enable_if_t<std::is_same<Ret, void>::value>>
    nullptr_t invoke(Tuple &&tuple) {
        constexpr size_t N = std::tuple_size<std::decay_t<Tuple>>::value;
        invokeImpl<Ret>(std::forward<Tuple>(tuple), std::make_index_sequence<N>{});
        return nullptr;
    }

    template <typename Ret, typename Tuple, size_t ...Is>
    Ret invokeImpl(Tuple &&tuple, std::index_sequence<Is...>) {
        return _func(std::get<Is>(std::forward<Tuple>(tuple))...);
    }

    std::decay_t<F> _func;
};

#endif