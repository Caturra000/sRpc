#ifndef __EPOLL_EXCEPTION_H__
#define __EPOLL_EXCEPTION_H__
#include "ErrnoException.h"
namespace mutty {

class EpollException: public ErrnoException {
    using ErrnoException::ErrnoException;
};

} // mutty
#endif