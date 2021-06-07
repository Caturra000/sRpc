#ifndef __SRPC_PROTOCOL_H__
#define __SRPC_PROTOCOL_H__
namespace srpc {
namespace protocol { // JSON-RPC 2.0

struct Field {
    constexpr static char jsonrpc[] {"jsonrpc"};
    constexpr static char method[]  {"method"};
    constexpr static char params[]  {"params"};
    constexpr static char result[]  {"result"};
    constexpr static char error[]   {"error"};
    constexpr static char id[]      {"id"};
    constexpr static char code[]    {"code"};
    constexpr static char message[] {"message"};
};

struct Attribute {
    constexpr static char version[] {"2.0"};
    constexpr static int parseErrorCode {-32700};
    constexpr static int invalidRequestCode {-32600};
    constexpr static int methodNotFoundCode {-32601};
    constexpr static int invalidParamsCode {-32602};
    constexpr static int internalErrorCode {-32603};

    constexpr static char parseError[] {"Parse error"};
    constexpr static char invalidRequest[] {"Invalid Request"};
    constexpr static char methodNotFound[] {"Method not found"};
    constexpr static char invalidParams[] {"Invalid params"};
    constexpr static char internalError[] {"Internal error"};
};

class Exception: public std::exception {
public:
    static Exception makeParseErrorException() {
        return {Attribute::parseErrorCode, Attribute::parseError};
    }
    static Exception makeInvalidRequestException() {
        return {Attribute::invalidRequestCode, Attribute::invalidRequest};
    }
    static Exception makeMethodNotFoundException() {
        return {Attribute::methodNotFoundCode, Attribute::methodNotFound};
    }
    static Exception makeInvalidParamsException() {
        return {Attribute::invalidParamsCode, Attribute::invalidParams};
    }
    static Exception makeInternalErrorException() {
        return {Attribute::internalErrorCode, Attribute::internalError};
    }

public:
    explicit Exception(int code): _code(code) {}
    Exception(int code, std::string message)
        : _code(code), _message(std::move(message)) {}
    const char* what() const noexcept override {
        return _verbose.c_str();
    }
    int code() const { return _code; }
    const std::string& message() const { return _message; }

private:
    int _code;
    std::string _message;
    const std::string _verbose{"srpc protocol exception: " + _message};
    mutty::Object data; // unused?
};

constexpr char Field::jsonrpc[];
constexpr char Field::method[];
constexpr char Field::params[];
constexpr char Field::result[];
constexpr char Field::error[];
constexpr char Field::id[];
constexpr char Field::code[];
constexpr char Field::message[];


constexpr char Attribute::version[];
constexpr char Attribute::parseError[];
constexpr char Attribute::invalidRequest[];
constexpr char Attribute::methodNotFound[];
constexpr char Attribute::invalidParams[];
constexpr char Attribute::internalError[];

} // protocol
} // srpc
#endif