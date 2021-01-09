#ifndef __RPC_CALL_PROXY_H__
#define __RPC_CALL_PROXY_H__
#include <bits/stdc++.h>
#include "vsjson/vsjson.hpp"
using namespace vsjson;


template <typename F>
class CallProxy {
public:

    CallProxy(F func): _func(std::move(func)) {}
    Json operator()(Json json) {
        return dispatch(_func, json);
    }

private:

    // 函数指针
    template <typename Ret, typename ...Args, 
    typename WrappedRet = std::conditional_t<
        std::is_same<Ret, void>::value, nullptr_t, Ret>>
    WrappedRet dispatch(Ret (*func)(Args...), Json &args) {
        using ArgsTuple = std::tuple<std::decay_t<Args>...>;
        constexpr size_t N = sizeof...(Args);
        // 从json构造tuple
        ArgsTuple argsTuple = make<ArgsTuple>(args, std::make_index_sequence<N>{});
        // 用tuple构造回原来的args，调用并返回
        WrappedRet result = invoke<Ret>(std::move(argsTuple)); 
        return result; // 如果为void，返回nullptr
    }

    // std::function
    template <typename Ret, typename ...Args, 
    typename WrappedRet = std::conditional_t<
        std::is_same<Ret, void>::value, nullptr_t, Ret>>
    WrappedRet dispatch(std::function<Ret(Args...)> &func, Json &args) {
        using ArgsTuple = std::tuple<std::decay_t<Args>...>;
        constexpr size_t N = sizeof...(Args);
        ArgsTuple argsTuple = make<ArgsTuple>(args, std::make_index_sequence<N>{});
        WrappedRet result = invoke<Ret>(argsTuple); 
        return result;
    }

    // 成员函数  FIXME 接口不一致
    // template <typename Ret, typename Class, typename ...Args, 
    // typename WrappedRet = std::conditional_t<
    //     std::is_same<Ret, void>::value, nullptr_t, Ret>>
    // WrappedRet dispatch(Ret (Class::*func)(Args...), Class *obj, 
    //                     Json &args, size_t len) {
    //     using ArgsTuple = std::tuple<std::decay_t<Args>...>;
    //     constexpr size_t N = sizeof...(Args);
    //     ArgsTuple argsTuple = make<ArgsTuple>(args, std::make_index_sequence<N>{});
    //     auto wrappedFunc = [=](Args... args) -> Ret { return (obj->*func)(args...); };
    //     WrappedRet result = invoke<Ret>(argsTuple); 
    //     return result;
    // }

    // TODO  lambda

    // 从json中处理整个tuple
    template <typename Tuple, size_t ...Is>
    Tuple make(Json &json, std::index_sequence<Is...>) {
        Tuple tuple;
        std::initializer_list<int> { (get<Tuple, Is>(json, tuple), 0)... };
        return tuple;
    }

    // 从json中处理单个elem
    template <typename Tuple, size_t I>
    void get(Json &from, Tuple &to) {
        using ElemType = std::decay_t<decltype(std::get<I>(to))>;
        std::get<I>(to) = std::move(from[I]).to<ElemType>();
    }

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

private:
    std::decay_t<F> _func;
    
};

#endif