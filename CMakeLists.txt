cmake_minimum_required(VERSION 3.7)
project(dreco)

set(SOURCE_LIST 
src/math/vec3.cxx
src/math/mat4.cxx
src/render/vulkan/vk_swapchain.cxx
src/render/vulkan/vk_renderer.cxx
src/engine/engine.cxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

add_library(${PROJECT_NAME} SHARED ${SOURCE_LIST})

find_package(SDL2 REQUIRED)

if (UNIX)
target_link_libraries(${PROJECT_NAME} SDL2 vulkan)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_subdirectory(examples/triangle)
