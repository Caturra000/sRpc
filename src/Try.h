#ifndef __SRPC_TRY_H__
#define __SRPC_TRY_H__
#include <bits/stdc++.h>
namespace srpc {

// TODO SFINAE: T == std::exception_ptr
template <typename T>
class Try {
public:
    Try(T value)
        : _tuple(std::make_tuple(std::move(value), std::exception_ptr())),
          _isValue(true) {}
    Try(std::exception_ptr e)
        : _tuple(std::make_tuple(T{}, std::move(e))),
          _isValue(false) {}
    Try(): Try(std::exception_ptr{}) {}

    bool isValue() { return _isValue; }
    bool isException() { return !_isValue; }

    T& value() { return std::get<0>(_tuple); }
    std::exception_ptr& exception() { return std::get<1>(_tuple); }

private:
    // TODO placement new & union version
    // union {
    //     T                  _value;
    //     std::exception_ptr _exception;
    // };

    std::tuple<T, std::exception_ptr> _tuple;

    bool _isValue; // is value
};

} // srpc
#endif