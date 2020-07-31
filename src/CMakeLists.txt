cmake_minimum_required(VERSION 3.10)
project(GOMOKU)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -O3 -std=c++11 -g")

add_executable(code mango.cpp)
