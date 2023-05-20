cmake_minimum_required(VERSION 3.19.0)
project(dreco-project)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(launcher)