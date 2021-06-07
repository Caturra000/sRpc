#ifndef __SRPC_PROTOCOL_H__
#define __SRPC_PROTOCOL_H__
namespace srpc {

// JSON-RPC 2.0
struct Protocol {
    constexpr static char jsonrpc[] {"jsonrpc"};
    constexpr static char version[] {"2.0"};
    constexpr static char method[]  {"method"};
    constexpr static char params[]  {"params"};
    constexpr static char result[]  {"result"};
    constexpr static char error[]   {"error"};
    constexpr static char id[]      {"id"};
};

constexpr char Protocol::jsonrpc[];
constexpr char Protocol::version[];
constexpr char Protocol::method[];
constexpr char Protocol::params[];
constexpr char Protocol::result[];
constexpr char Protocol::error[];
constexpr char Protocol::id[];

} // srpc
#endif