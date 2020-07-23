cmake_minimum_required(VERSION 3.7)
project(triangle)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

target_link_libraries(${PROJECT_NAME} dreco)

set(BINARY_SHADER_LIST 
${PROJECT_BINARY_DIR}/shaders/frag.spv
${PROJECT_BINARY_DIR}/shaders/vert.spv)

install(FILES ${BINARY_SHADER_LIST} DESTINATION ${DRECO_INSTALL_DIR}/shaders)