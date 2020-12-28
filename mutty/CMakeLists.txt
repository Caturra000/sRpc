cmake_minimum_required(VERSION 3.0.0)
project(mutty VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(Mutty SHARED Mutty.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include_directories(src)