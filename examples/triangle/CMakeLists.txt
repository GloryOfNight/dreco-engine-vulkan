cmake_minimum_required(VERSION 3.16.2)
project(triangle)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco)

if (MSVC)
## Copy binary files into executable folder
get_target_property(DRECO_BINARY_DIR dreco BINARY_DIR)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} 
-E copy_if_different "${DRECO_BINARY_DIR}/dreco.dll" ${PROJECT_BINARY_DIR})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} 
-E copy_if_different "${SDL2_DLL}" ${PROJECT_BINARY_DIR})
endif()