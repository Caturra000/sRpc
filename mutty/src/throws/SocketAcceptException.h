#ifndef __SOCKET_ACCEPT_EXCEPTION_H__
#define __SOCKET_ACCEPT_EXCEPTION_H__
#include "SocketException.h"
class SocketAcceptException: public SocketException {
public:
    static constexpr const char *TAG = "socket accept exception";
    using SocketException::SocketException;
    SocketAcceptException(int err): SocketException(TAG, err) { }
};
#endif