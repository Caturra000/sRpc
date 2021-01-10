#ifndef __RPC_POINT_H__
#define __RPC_POINT_H__
#include "vsjson.hpp"
using namespace vsjson;
struct Point {
    int x, y, z;
    operator Json() { return Json::array(x, y, z); }
    Point(int x, int y, int z): x(x), y(y), z(z) {}
    Point(const std::vector<vsjson::Json> &array)
        : x(array[0].to<int>()),
          y(array[1].to<int>()),
          z(array[2].to<int>()) {}
    Point() = default;
};
#endif