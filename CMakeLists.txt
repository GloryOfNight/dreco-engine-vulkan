cmake_minimum_required(VERSION 3.19.0)
project(dreco-project)

include(cmake/settings.cmake)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(launcher)