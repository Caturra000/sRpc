#ifndef __READ_EXCEPTION_H__
#define __READ_EXCEPTION_H__
#include "IoException.h"
class ReadException: public IoException {
public:
    static constexpr const char *TAG = "read exception";
    using IoException::IoException;
    ReadException(int err): IoException(TAG, err) { }
};
#endif