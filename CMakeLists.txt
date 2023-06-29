cmake_minimum_required(VERSION 3.23.0)
project(dreco-project)

include(cmake/settings.cmake)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(launcher)