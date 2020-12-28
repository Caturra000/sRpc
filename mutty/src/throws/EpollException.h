#ifndef __EPOLL_EXCEPTION_H__
#define __EPOLL_EXCEPTION_H__
#include "ErrnoException.h"
class EpollException: public ErrnoException {
    using ErrnoException::ErrnoException;
};
#endif