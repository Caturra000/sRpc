#ifndef __UTILS_TIMESTAMP_H__
#define __UTILS_TIMESTAMP_H__
#include <chrono>
namespace mutty {

// using std::chrono::system_clock;
using namespace std::literals::chrono_literals;


using Nanosecond = std::chrono::nanoseconds;
using Microsecond = std::chrono::microseconds;
using Millisecond = std::chrono::milliseconds;
using Second = std::chrono::seconds;
using Minute = std::chrono::minutes;
using Hour = std::chrono::hours;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, Nanosecond>;

Timestamp now() { return std::chrono::system_clock::now(); }
Timestamp nowAfter(Nanosecond interval) { return now() + interval; }
Timestamp nowBefore(Nanosecond interval) { return now() - interval; }

} // mutty
#endif