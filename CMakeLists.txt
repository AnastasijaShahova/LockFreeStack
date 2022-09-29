cmake_minimum_required(VERSION 3.5)

project(LockFreeStack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LockFreeStack main.cpp LockFreeStack.h)

install(TARGETS LockFreeStack
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
