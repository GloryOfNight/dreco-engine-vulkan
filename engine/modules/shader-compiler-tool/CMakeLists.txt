# Dreco tool for shader compiling in runtime

project(dreco-shader-compiler-tool)

add_library(${PROJECT_NAME} STATIC src/shader_compiler.cxx)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
    EXPORT_COMPILE_COMMANDS TRUE
    POSITION_INDEPENDENT_CODE TRUE
)