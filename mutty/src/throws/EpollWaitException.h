#ifndef __EPOLL_WAIT_EXCEPTION_H__
#define __EPOLL_WAIT_EXCEPTION_H__
#include "EpollException.h"
class EpollWaitException: public EpollException {
public:
    static constexpr const char *TAG = "epoll create exception";
    using EpollException::EpollException;
    EpollWaitException(int err): EpollException(TAG, err) { }
};
#endif