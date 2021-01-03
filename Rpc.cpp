#include <bits/stdc++.h>

#include "vsJSON/VsJson.cpp"
#include "mutty/Mutty.cpp"
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
    template <typename Tuple, size_t ...I>
    Tuple make(const char *&from, std::index_sequence<I...>) {
        Tuple tuple;
        std::initializer_list<int> { (get<Tuple, I>(from, tuple), 0)... };
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

    template <typename Ret, typename Tuple, size_t ...I>
    Ret invokeImpl(Tuple &&tuple, std::index_sequence<I...>) {
        return _func(std::get<I>(std::forward<Tuple>(tuple))...);
    }

    std::decay_t<F> _func;
};

class Codec {

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


class RpcServer {

};

class RpcClient {
    Looper looper;
    Client client; // single thread
    int idGen;
    std::map<int, Json> mj; // 需要lock? 不需要吧
    // void test() {
        // results.back().
    // }


    // 需求1： 不能阻塞send过程，也就是send和process不能合在一块，不然影响server对单一client的效率
    // 需求2： 希望过程是透明的，返回std::future<T>而不是Json

    // template <typename T>
    std::future<int/*T*/> call(int arg) {
        auto promise = std::make_shared<std::promise<Json>>();
        auto retPromise = std::make_shared<std::promise<int>>();
        auto scheduler = looper.getScheduler();
        int token; // idGen
        scheduler->runAt(now()).with([this, /*MoveWrapper, */ promise, retPromise, token] {
            // client.send(); // 只是写到app buffer中，足够快
            // promise咋办？
            // ps.push(std::move(promise));
            // promise怎么泛型擦除，不擦了，用JSON

            // 再多加一个异步处理过程

            // then 严格顺序
            looper.getScheduler()->runAt(now()).with([this, token] { // <T>
                if(!mj.count(token)) ; // retry
            });
        });
        
        return retPromise->get_future();
    }

    template <int Cur, typename Arg, typename ...Args>
    void createImpl(Json &json, Arg &&arg, Args &&...args) {
        if(sizeof...(Args) == 0) return;
        json["arg" + std::to_string(Cur) ] = arg;
        createImpl<Cur+1>(json, td::forward<Args>(args)...);
    }

    template <typename ...Args>
    Json create(Args &&...args) {
        Json json = 
        {
            {"args", nullptr}
        };
        Json &argsJson = json["args"];
        createImpl<0>(argsJson, std::forward<Args>(args)...);
        return json;
    }

    template <typename T, typename ...Args> // PODs
    std::future<T> callT(Args &&...args) {
        Json json = create(std::forward<Args>(args)...);
        auto promise = std::make_shared<std::promise<T>>();
        auto scheduler = looper.getScheduler();
        scheduler->runAt(now()).with([this, /*MoveWrapper, */ promise] {
            int token = ++idGen; // idGen
            json["token"] = token;
            // client.send(/*json.dump()*/);
            looper.getScheduler()->runAt(now()).with([this, token, promise] { // <T>
                if(!mj.count(token)) ; // retry
                // promise->set_value(/*json*/); // 只有一个值的json
            });
        });
        
        return promise->get_future();
    }

    RpcClient(): client(&looper, InetAddress("127.0.0.1", 23333)) {
        client.onMessage([] { // 类型丢失，惨
            // if(codec.valid()) {
                // token添加对应json

            // }

            // 既然类型必然丢失，那只能传JSON，不能在这set_value<T>
        });
    }
};


int main() {
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


    RpcService rpc;
    rpc.bind("add", add);
    std::function<int(int, int)> func = add;
    rpc.bind("func", func);
    auto r = rpc.call<int>("add", 1, 2);
    std::cout << r;
    std::cout << std::endl;
    
    Looper looper;
    Server server(&looper, InetAddress("127.0.0.1", 23333));

    Codec codec;
    server.onMessage([&codec](TcpContext *ctx) {
        // if(codec.valid())
    });
    
    
}
