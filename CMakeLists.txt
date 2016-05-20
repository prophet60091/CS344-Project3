cmake_minimum_required(VERSION 3.3)
project(Project3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES marcel.c)
add_executable(Project3 ${SOURCE_FILES})