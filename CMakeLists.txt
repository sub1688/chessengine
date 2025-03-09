cmake_minimum_required(VERSION 3.29)
project(chessengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


set(VULKAN_SDK "C:/VulkanSDK/1.4.304.1")
message("${VULKAN_SDK}")

set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include")
set(Vulkan_LIBRARY "${VULKAN_SDK}/Lib/vulkan-1.lib")
message(${Vulkan_INCLUDE_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(Walnut-cmake)
include_directories(chessengine)

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
)

target_link_libraries(chessengine PRIVATE Walnut ${Vulkan_LIBRARIES} glfw ${OPENGL_gl_LIBRARY})

