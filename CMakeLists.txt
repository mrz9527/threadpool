cmake_minimum_required(VERSION 3.5)
project(threadpool)

set(CMAKE_CXX_STANDARD 11)

add_executable(threadpool main.cpp CThreadPool.cpp)
target_link_libraries(threadpool pthread)