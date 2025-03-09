cmake_minimum_required(VERSION 3.0.0)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/Walnut/src)

link_libraries(glfw)
link_libraries(imgui)
link_libraries(glm)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Walnut)
