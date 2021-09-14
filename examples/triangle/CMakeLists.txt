cmake_minimum_required(VERSION 3.16.2)
project(triangle)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco)

get_target_property(DRECO_BINARY_DIR dreco BINARY_DIR)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE
    RUNTIME_OUTPUT_DIRECTORY ${DRECO_BINARY_DIR}
)
