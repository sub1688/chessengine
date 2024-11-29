cmake_minimum_required(VERSION 3.29)
project(chessengine)

set(CMAKE_CXX_STANDARD 20)

include_directories(chessengine)

add_executable(chessengine main.cpp
        board.h
        board.cpp
        movegen.h
        movegen.cpp)
target_link_libraries(chessengine)
