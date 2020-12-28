#ifndef __WRITE_EXCEPTION_H__
#define __WRITE_EXCEPTION_H__
#include "IoException.h"
class WriteException: public IoException {
public:
    static constexpr const char *TAG = "write exception";
    using IoException::IoException;
    WriteException(int err): IoException(TAG, err) { }
};
#endif