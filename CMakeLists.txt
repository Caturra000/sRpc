cmake_minimum_required(VERSION 3.0.0)
project(rpcService VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(vsjson)
add_subdirectory(mutty)

include_directories(INCLUDE_DIR ${INCLUDE_DIR} src)
include_directories(INCLUDE_DIR ${INCLUDE_DIR} mutty/src)
include_directories(INCLUDE_DIR ${INCLUDE_DIR} vsjson/src)

add_executable(rpc Rpc.cpp)