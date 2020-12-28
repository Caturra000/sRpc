#ifndef __BUFFER_H__
#define __BUFFER_H__
#include <sys/uio.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include "utils/Algorithms.h"
#include "net/Socket.h"
// 一个简单的Socket IO Buffer
// 提供char*兼容，以及尽可能延迟扩容
class Buffer {
public:
    Buffer(int size = 128): _capacity(size), _buf(size), _r(0), _w(0) { }
    
private:
    std::vector<char> _buf;
    int _capacity;
    int _r, _w; // _r <= _w

public:

// 大小接口

    int size() { return _capacity; } // 不区分size和capacity
    int unread() { return _w - _r; }  // 还有多少未读取
    int unwrite() { return _capacity - _w; } // 还有多少彻底填满
    int available() { return _capacity - unread(); } // 不考虑分区计算可用空间

// 扩容相关

    void clear() { _r = _w = 0; }

    // 能否复用，尽量延迟resize
    // TODO 当unread()为一个极少占比的时候，尝试copy
    void reuseIfPossible();

    // 处理边缘碎片
    void gc();

    // 尽可能延迟resize
    // 根据hint来决定gc策略 / 是否gc
    void gc(int hint);

    // TODO expand不会有额外check，应该放到private

    void expand() { _buf.resize(_capacity <<= 1); }
    void expandTo(int size) { _buf.resize(_capacity = size); }


// IO相关

    void read(int n) { 
        _r += n; 
    }
    void write(int n) {  // 实际write后调用
        _w += n; 
        reuseIfPossible();
    }

    char* readBuffer() { return _buf.data() + _r; }
    char* writeBuffer() { return _buf.data() + _w; }
    const char* end() { return _buf.data() + _capacity; }

    void append(const char *data, int size) {
        gc(size);
        memcpy(writeBuffer(), data, size);
        write(size);
    }

    // not a pointer
    template <typename T, typename = std::enable_if_t<!std::is_pointer<T>::value>>
    void append(const T &data) { append((const char *)(&data), sizeof(T)); }
    
    template <typename T, size_t N>
    void append(const T (&data)[N]) { append(data, sizeof(T) * N); }


// syscall

    int readFrom(const Socket &socket) { return readFrom(socket.fd()); }
    int readFrom(int fd);
    int writeTo(int fd) {
        int n = ::write(fd, readBuffer(), unread());
        read(n);
        // n < 0 !EAGAIN throw
        return n;
    }

// simple output

    friend std::ostream& operator << (std::ostream &os, Buffer &buf) {
        auto iter = buf.readBuffer();
        const auto bound = buf.writeBuffer();
        while(iter != bound) os << *iter++;
        return os;
    }
};


inline void Buffer::reuseIfPossible() {
    if(_r == _w) {
        clear();
    }
    int bufferRest = unread();
    if(bufferRest < (_capacity >> 4) && bufferRest < 32) {
        // TODO
    }
}

inline void Buffer::gc() {
    if(_r > 0) {
        // move [_r, _w) to front
        memmove(_buf.data(), _buf.data() + _r, _w = unread());
        _r = 0;
    }
}

inline void Buffer::gc(int hint) {
    if(hint <= unwrite()) return;
    gc();
    // still cannot store
    if(hint > available()) {
        int appendSize =  hint - available();
        int expectSize = _buf.size() + appendSize;
        expandTo(/*_capacity = */roundToPowerOfTwo(expectSize));
    }
}

inline int Buffer::readFrom(int fd) {
    char localBuffer[1<<16];
    iovec vec[2];
    int bufferLimit = unwrite();
    vec[0].iov_base = writeBuffer();
    vec[0].iov_len = bufferLimit;
    vec[1].iov_base = localBuffer;
    vec[1].iov_len = sizeof(localBuffer);
    int n = ::readv(fd, vec, 2);
    if(n > 0) {
        if(n <= bufferLimit) {
            write(n);
        } else {
            write(bufferLimit);
            int localSize = n - bufferLimit;
            append(localBuffer, localSize);
        }
    }
    // n < 0 error
    return n;
}
#endif