#ifndef __RPC_RPC_SERVER_H__
#define __RPC_RPC_SERVER_H__
#include <bits/stdc++.h>
#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
#include "CallProxy.h"
#include "Codec.h"
using namespace mutty;
using namespace vsjson;


class RpcServer {
public:
    template <typename F>
    void bind(const std::string &name, const F &func) {
        _table[name] = CallProxy<F>(func);
    }

    // args是JsonArray类型
    Json netCall(const std::string &name, Json args) {
        return _table[name](std::move(args));
    }
private:
    std::map<std::string, std::function<Json(Json)>> _table;
};
#endif