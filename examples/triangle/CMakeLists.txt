cmake_minimum_required(VERSION 3.16.2)
project(triangle)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

target_link_libraries(${PROJECT_NAME} dreco)

set(BINARY_SHADER_LIST 
"${CMAKE_SOURCE_DIR}/shaders/frag.spv"
"${CMAKE_SOURCE_DIR}/shaders/vert.spv")

set(CONTENT_DIR
"${CMAKE_SOURCE_DIR}/content")

file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/shaders)
file(COPY "${CMAKE_SOURCE_DIR}/shaders" DESTINATION ${PROJECT_BINARY_DIR}/) 

file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/content)
file(COPY "${CMAKE_SOURCE_DIR}/content" DESTINATION ${PROJECT_BINARY_DIR}/) 

install(FILES ${BINARY_SHADER_LIST} DESTINATION ${CMAKE_INSTALL_RPATH}/shaders)
install(DIRECTORY ${CONTENT_DIR} DESTINATION ${CMAKE_INSTALL_RPATH})

if (MSVC)
get_target_property(DRECO_BINARY_DIR dreco BINARY_DIR)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} 
-E copy_if_different "${DRECO_BINARY_DIR}/dreco.dll" ${PROJECT_BINARY_DIR})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} 
-E copy_if_different "${SDL2_DLL}" ${PROJECT_BINARY_DIR})
endif()