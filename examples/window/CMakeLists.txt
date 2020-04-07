cmake_minimum_required(VERSION 3.7)
project(window)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco)

file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${PROJECT_BINARY_DIR}/)