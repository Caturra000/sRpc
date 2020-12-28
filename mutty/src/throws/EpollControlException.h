#ifndef __EPOLL_CONTROL_EXCEPTION_H__
#define __EPOLL_CONTROL_EXCEPTION_H__
#include "EpollException.h"
class EpollControlException: public EpollException {
public:
    static constexpr const char *TAG = "epoll control exception";
    using EpollException::EpollException;
    EpollControlException(int err): EpollException(TAG, err) { }
};
#endif