cmake_minimum_required(VERSION 3.8)

file(GLOB imgui_headers ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_internal.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/imstb_rectpack.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/imstb_textedit.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/imstb_truetype.h)

file(GLOB imgui_sources ${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp)

project(imgui)
add_library(${PROJECT_NAME} STATIC ${imgui_headers} ${imgui_sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
