cmake_minimum_required(VERSION 2.8)
project(blud2e)
include_directories(.)

set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS} -lz -g -ftest-coverage -fprofile-arcs")
set(SOURCE_EXE blud2e.cpp io.cpp conver.cpp obj.cpp info.cpp)
set(HEADER blud2e.h)

add_executable(blud2e ${SOURCE_EXE})

