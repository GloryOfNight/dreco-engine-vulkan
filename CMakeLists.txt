cmake_minimum_required(VERSION 3.7)
project(dreco)

set(SOURCE_LIST 
src/math/vec3.cxx
src/math/mat4.cxx
src/render/vulkan/vk_swapchain.cxx
src/render/vulkan/vk_renderer.cxx
src/engine/engine.cxx)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_library(${PROJECT_NAME} SHARED ${SOURCE_LIST})

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

if (UNIX)
target_link_libraries(${PROJECT_NAME} vulkan)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_subdirectory(examples/triangle)
