#ifndef __SOCKET_CREATE_EXCEPTION_H__
#define __SOCKET_CREATE_EXCEPTION_H__
#include "SocketException.h"
class SocketCreateException: public SocketException {
public:
    static constexpr const char *TAG = "socket create exception";
    using SocketException::SocketException;
    SocketCreateException(int err)
        : SocketException(TAG, err) { }
};
#endif