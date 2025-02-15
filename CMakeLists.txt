cmake_minimum_required(VERSION 3.29)
project(chessengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

set(SFML_USE_STATIC_STD_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

include_directories(chessengine)

add_executable(chessengine main.cpp
        engine/board.h
        engine/board.cpp
        engine/movegen.h
        engine/movegen.cpp
        ui/window.h
        ui/window.cpp
        engine/search.h
        engine/search.cpp
        engine/piecesquaretable.h
        engine/piecesquaretable.cpp
        util/arrayvec.h
        engine/old/oldsearch.h
        engine/old/oldsearch.cpp
        engine/zobrist.h
        engine/zobrist.cpp
        engine/transpositiontable.h
        engine/transpositiontable.cpp
        engine/san.h
        engine/san.cpp)

target_link_libraries(chessengine PRIVATE sfml-graphics sfml-window sfml-system)

