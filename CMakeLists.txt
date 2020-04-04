cmake_minimum_required(VERSION 3.7)
project(dreco)

set(SOURCE_LIST src/engine/engine.cxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ")

add_library(${PROJECT_NAME} ${SOURCE_LIST})

find_package(glfw3 REQUIRED)

if (UNIX)
target_link_libraries(${PROJECT_NAME} glfw)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/src/)

if (BUILD_EXAMPLES) 
add_subdirectory(examples/window)
endif()