cmake_minimum_required(VERSION 3.29)
project(chessengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(IMGUI_SOURCES imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp)

find_package(OpenGL REQUIRED)
add_subdirectory(glfw)

include_directories(chessengine imgui)

add_executable(chessengine main.cpp
        engine/board.h
        engine/board.cpp
        engine/movegen.h
        engine/movegen.cpp
        engine/search.h
        engine/search.cpp
        engine/piecesquaretable.h
        engine/piecesquaretable.cpp
        util/arrayvec.h
        engine/zobrist.h
        engine/zobrist.cpp
        engine/transpositiontable.h
        engine/transpositiontable.cpp
        engine/san.h
        engine/san.cpp
        engine/openingbook.h
        engine/openingbook.cpp
        ui/gui.h
        ui/gui.cpp
        ${IMGUI_SOURCES}
)

target_link_libraries(chessengine PRIVATE OpenGL::GL glfw)

