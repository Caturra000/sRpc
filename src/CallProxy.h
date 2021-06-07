#ifndef __SRPC_CALL_PROXY_H__
#define __SRPC_CALL_PROXY_H__
#include <bits/stdc++.h>
#include "vsjson.hpp"
#include "FunctionTraits.h"
#include "Protocol.h"
namespace srpc {

template <typename F>
class CallProxy {
public:
    CallProxy(F func): _func(std::move(func)) {}
    vsjson::Json operator()(vsjson::Json json) { return dispatch(json); }

private: /// IMPL
    template <typename WrappedRet = std::conditional_t<
        std::is_same<typename FunctionTraits<F>::ReturnType, void>::value,
            nullptr_t, typename FunctionTraits<F>::ReturnType>>
    WrappedRet dispatch(vsjson::Json &args) {
        using Ret = typename FunctionTraits<F>::ReturnType;
        using ArgsTuple = typename FunctionTraits<F>::ArgsTuple;
        constexpr size_t N = FunctionTraits<F>::ArgsSize;
        if(N != args.arraySize()) {
            throw protocol::Exception::makeInvalidParamsException();
        }
        // 从json构造tuple
        ArgsTuple argsTuple = make<ArgsTuple>(args, std::make_index_sequence<N>{});
        // 用tuple构造回原来的args，调用并返回
        WrappedRet result = invoke<Ret>(std::move(argsTuple));
        return result; // 如果为void，返回nullptr
    }

    // 从json中处理整个tuple
    template <typename Tuple, size_t ...Is>
    Tuple make(vsjson::Json &json, std::index_sequence<Is...>) {
        Tuple tuple;
        std::initializer_list<int> { (get<Tuple, Is>(json, tuple), 0)... };
        return tuple;
    }

    // 从json中处理单个elem
    template <typename Tuple, size_t I>
    void get(vsjson::Json &from, Tuple &to) {
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

} // srpc
#endif