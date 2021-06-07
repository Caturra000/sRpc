#ifndef __SRPC_RPC_FIELD_H__
#define __SRPC_RPC_FIELD_H__
namespace srpc {

struct RpcField {
    constexpr const static char *NAME = "name";
    constexpr const static char *ARGS = "args";
    constexpr const static char *TOKEN = "token";
    constexpr const static char *RETURN = "return";
};

} // srpc
#endif