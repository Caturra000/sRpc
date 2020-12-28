#ifndef __EPOLL_CREATE_EXCEPTION_H__
#define __EPOLL_CREATE_EXCEPTION_H__
#include "EpollException.h"
class EpollCreateException: public EpollException {
public:
    static constexpr const char *TAG = "epoll create exception";
    using EpollException::EpollException;
    EpollCreateException(int err): EpollException(TAG, err) { }
};
#endif