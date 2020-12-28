#ifndef __IO_EXCEPTION_H__
#define __IO_EXCEPTION_H__
#include "ErrnoException.h"
class IoException: public ErrnoException {
    using ErrnoException::ErrnoException;
};
#endif