cmake_minimum_required(VERSION 3.0.0)
project(shader_list_generator)

add_executable(${PROJECT_NAME} main.cxx)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
    EXPORT_COMPILE_COMMANDS TRUE
)