#ifndef __NETWORK_EXCEPTION_H__
#define __NETWORK_EXCEPTION_H__
#include "ErrnoException.h"
class NetworkException: public ErrnoException {
    using ErrnoException::ErrnoException;
};
#endif