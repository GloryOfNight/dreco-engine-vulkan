cmake_minimum_required(VERSION 3.0.0)
project(shader-compiler)

if (BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME} main.cxx shader_compiler.cxx)
    get_target_property(DRECO_BINARY_DIR dreco BINARY_DIR)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        EXPORT_COMPILE_COMMANDS TRUE
        RUNTIME_OUTPUT_DIRECTORY ${DRECO_BINARY_DIR})
else()
    add_library(${PROJECT_NAME}-static STATIC shader_compiler.cxx include/shader_compiler.hxx)
    target_include_directories(${PROJECT_NAME}-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
    set_target_properties(${PROJECT_NAME}-static PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        EXPORT_COMPILE_COMMANDS TRUE)
    target_compile_definitions(${PROJECT_NAME}-static PRIVATE BUILD_STATIC)
endif()