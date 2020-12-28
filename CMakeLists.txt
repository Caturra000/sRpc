cmake_minimum_required(VERSION 3.0.0)
project(rpcService VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(rpc Rpc.cpp)
target_link_libraries(rpc Mutty vsJSON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(vsJSON)
add_subdirectory(mutty)

include_directories(INCLUDE_TEST src)
include_directories(INCLUDE_TEST ${INCLUDE_TEST} mutty/src)
include_directories(INCLUDE_TEST ${INCLUDE_TEST} vsJSON/src)