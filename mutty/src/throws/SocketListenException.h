#ifndef __SOCKET_LISTEN_EXCEPTION_H__
#define __SOCKET_LISTEN_EXCEPTION_H__
#include "SocketException.h"
class SocketListenException: public SocketException {
public:
    static constexpr const char *TAG = "socket listen exception";
    using SocketException::SocketException;
    SocketListenException(int err): SocketException(TAG, err) { }
};
#endif