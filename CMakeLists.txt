cmake_minimum_required(VERSION 3.16.2)
project(dreco-project)

set(DRECO_BUILD_SHARED TRUE)

set(DRECO_ASSETS_DIR "assets")
set(DRECO_SHADERS_SOURCE_DIR "shaders/src")
set(DRECO_SHADERS_BINARY_DIR "shaders/bin")

add_subdirectory(game)
add_subdirectory(engine)
add_subdirectory(launcher)

install(DIRECTORY ${DRECO_ASSETS_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${DRECO_SHADERS_BINARY_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)