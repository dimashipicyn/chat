cmake_minimum_required(VERSION 3.0.0)
project(chat)

add_compile_options(-Wall -Wextra -pedantic)

set (CMAKE_CXX_STANDARD 17)

add_executable(server
    server.cpp
    server.hpp
)
