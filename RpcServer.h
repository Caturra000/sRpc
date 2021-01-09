#ifndef __RPC_RPC_SERVER_H__
#define __RPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
using namespace mutty;
using namespace vsjson;


struct StreamVisitor {
    using ReturnType = std::pair<const void*, int>;
    // 由于构造和template上的麻烦，rpc函数尽量避免接收类似string的参数
    ReturnType operator()(const std::string &obj) {
        return {obj.c_str(), obj.size() + 1}; // +'\0'
    }

    template <typename T>
    ReturnType operator()(const T &obj) {
        return {&obj, sizeof(obj)};
    }

    // TODO vector/map throw
};

class RpcService {
public:
    template <typename F>
    void bind(const std::string &name, const F &func) {
        _table[name] = CallProxy<F>(func);
    }

    template <typename Ret, typename ...Args>
    Ret call(const std::string &name, Args ...args) {
        // HARD CODE
        char buf[1234]{};
        return _table[name](buf, sizeof(int)*2).as<Ret>();
    }

    Json remoteCall(const std::string &name, const Json &args) {
        char buf[65536];
        int len = 0;
        StreamVisitor sv;
        for(int i = 0, N = args.arraySize(); i < N; ++i) {
            auto &value = const_cast<Json&>(args[i]).value();
            auto piece = value.visit(sv);
            memcpy(buf+len, piece.first, piece.second);
            len += piece.second;
        }
        return _table[name](buf, len);
    }

private:
    std::map<std::string, std::function<Json(const char*, size_t)>> _table;
};


class RpcServer {

};
#endif