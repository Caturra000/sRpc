#ifndef __SOCKET_EXCEPTION_H__
#define __SOCKET_EXCEPTION_H__
#include "NetworkException.h"
class SocketException: public NetworkException {
public:
    static constexpr const char *TAG = "socket exception";
    using NetworkException::NetworkException;
    SocketException(int err): NetworkException(TAG, err) { }
};
#endif