cmake_minimum_required(VERSION 3.0.0)
project(rpcService VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


include_directories(INCLUDE_DIR ${INCLUDE_DIR} src)
include_directories(INCLUDE_DIR ${INCLUDE_DIR} depends/mutty depends/mutty/src)
include_directories(INCLUDE_DIR ${INCLUDE_DIR} depends/vsjson depends/vsjson/src)

add_executable(test_server test_server.cpp)
add_executable(test_client test_client.cpp)
