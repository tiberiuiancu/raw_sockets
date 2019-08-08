cmake_minimum_required(VERSION 3.14)
project(L4)

set(CMAKE_CXX_STANDARD 14)

add_executable(L4 client.cpp server.cpp client_delete.cpp)