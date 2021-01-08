#include <bits/stdc++.h>

#include "mutty/mutty.hpp"
#include "vsjson/vsjson.hpp"
using namespace mutty;
using namespace vsjson;

template <typename F>
class CallProxy {
public:
    
    CallProxy(F func): _func(std::move(func)) {}
    Json operator()(const char* stream, size_t len) {
        const char *backup = stream;
        auto result = dispatch(_func, stream, len); // FIXME 返回引用会decay
        return result; // 隐式转换
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

    // 从stream中处理单个elem
    template <typename Tuple, size_t I>
    void get(const char *&from, Tuple &to) {
        constexpr size_t size = sizeof(std::get<I>(to));
        memcpy(std::addressof(std::get<I>(to)), from, size);
        from += size;
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

    std::decay_t<F> _func;
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
private:
    std::map<std::string, std::function<Json(const char*, size_t)>> _table;
};


int add(int a, int b) {
    return a+b;
}

void minus(int a, int b) {
    
}


class Codec {
public:
    bool verify(Buffer &buffer) {
        if(buffer.unread() < sizeof(uint32_t)) return false;
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ntohl(beLength);
        return buffer.unread() >= sizeof(uint32_t) + length;
    }

    Json decode(Buffer &buffer) {
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ntohl(beLength);
        buffer.read(sizeof(uint32_t));
        // char ch;
        // bool overload = buffer.readBuffer() + length != buffer.end();
        // if(overload) {
        //     char &backup = buffer.readBuffer()[length];
        //     ch = backup;
        //     backup = '\0';
        // }
        Json json = parse(/*std::string(*/buffer.readBuffer()/*, length)*/);
        // if(overload) {
        //     buffer.readBuffer()[length] = ch;
        // }
        buffer.read(length);
        return json;
    }
};

class RpcServer {

};

class RpcClient {
public:
    template <typename T>
    void trySetValue(int token, std::shared_ptr<std::promise<T>> promise) {
        auto iter = records.find(token);
        if(iter != records.end()) {
            promise->set_value(std::move(iter->second).to<T>());
            records.erase(iter);
        } else {
            looper.getScheduler()->runAt(now())
                .with([=] {trySetValue(token, promise);}); // retry
        }
    }

    template <typename T, typename ...Args> // PODs
    std::future<T> call(Args &&...args) {
        auto request = std::make_shared<Json>(create(std::forward<Args>(args)...));
        auto promise = std::make_shared<std::promise<T>>();
        looper.getScheduler()->runAt(now()).with([this, request, promise] {
            int token = ++idGen;
            (*request)["token"] = token;
            std::string dump = request->dump();
            uint32_t beLength = htonl(dump.length());
            // client.send(beLength);
            // client.send(/*dump*/);
            looper.getScheduler()->runAt(now())
                .with([=] {trySetValue(token, promise);});
        });
        return promise->get_future();
    }

    RpcClient(const InetAddress &serverAddress): client(&looper, serverAddress) {
        client.onMessage([this](TcpContext *ctx) {
            if(codec.verify(ctx->inputBuffer)) {
                Json response = codec.decode(ctx->inputBuffer);
                int token = response["token"].as<int>();
                records[token] = std::move(response["ret"]);
            }
        });
    }

private:
    template <typename ...Args>
    void createImpl(Json &json, Args &&...args) {
    }

    template <typename Arg, typename ...Args>
    void createImpl(Json &json, Arg &&arg, Args &&...args) {
        json.append(std::forward<Arg>(arg));
        createImpl(json, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    Json create(Args &&...args) {
        Json json = 
        {
            {"args", Json::array()}
        };
        Json &argsJson = json["args"];
        createImpl(argsJson, std::forward<Args>(args)...);
        return json;
    }

    Looper looper;
    Client client; // single thread
    int idGen;
    std::map<int, Json> records;
    Codec codec;
};


int main() try {
    std::cout << "o55" << std::endl;
    // std::function<int(int, int)> func = add;

    // CallProxy<decltype(add)> cf = add;
    // CallProxy<decltype(func)> cf_func = func;
    // CallProxy<decltype(minus)> mn = minus;
    // char buf[sizeof(1)+sizeof(2)];
    // cf(buf, sizeof(1)+sizeof(2));
    // cf_func(buf, sizeof(1)+sizeof(2));;
    // std::function<void(const char*, size_t)> f = CallProxy<decltype(add)>(add);
    // Service service;
    // service.bind("add", add);
    // service.bind("min", minus);
    // Test t;
    // auto memberFunc = &Test::add;
    // auto lambdaFunc = [&](int,int)->int { return 1; };

    auto lam = [](int n) {
        std::function<int(int)> lam2;
        lam2 = [&] (int n) {
            return n == 1 ? 1 : n + lam2(n-1);
        };
        return lam2(n);
        
    };
    std::cout << lam(3) << std::endl;

    RpcService rpc;
    rpc.bind("add", add);
    std::function<int(int, int)> func = add;
    rpc.bind("func", func);
    auto r = rpc.call<int>("add", 1, 2);
    std::cout << r;
    std::cout << std::endl;
    
    RpcClient client(InetAddress("127.0.0.1", 23333));
    auto future = client.call<int>("add", 1, 2);
    auto result = future.get();
    //auto pr = std::make_shared<std::promise<int>>();
    // client.trySetValue(1, pr);
    
} catch (ErrnoException e) {
    std::cerr << e.errorCode() << " " << e.errorMessage() << std::endl;
}
