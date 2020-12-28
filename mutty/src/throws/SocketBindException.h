#ifndef __SOCKET_BIND_EXCEPTION_H__
#define __SOCKET_BIND_EXCEPTION_H__
#include "SocketException.h"
class SocketBindException: public SocketException {
public:
    static constexpr const char *TAG = "socket bind exception";
    using SocketException::SocketException;
    SocketBindException(int err): SocketException(TAG, err) { }
};
#endif