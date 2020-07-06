cmake_minimum_required(VERSION 3.7)
project(triangle)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

target_link_libraries(${PROJECT_NAME} dreco)

file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${PROJECT_BINARY_DIR}/)

file(REMOVE_RECURSE ${DRECO_INSTALL_DIR}/shaders)
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${DRECO_INSTALL_DIR}/)