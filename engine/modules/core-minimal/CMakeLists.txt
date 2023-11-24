# Dreco core files
# includes logging, paths and macroses needed for all dreco projects
# should be minimal as much as possible

project(dreco-core-minimal)

file(GLOB_RECURSE DRECO_CORE_MINIMAL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)
file(GLOB_RECURSE DRECO_CORE_MINIMAL_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

add_library(${PROJECT_NAME} STATIC ${DRECO_CORE_MINIMAL_SOURCE_FILES} ${DRECO_CORE_MINIMAL_HEADER_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
-DDRECO_ASSETS_DIR="${DRECO_ASSETS_DIR}"
-DDRECO_SHADERS_SOURCE_DIR="${DRECO_SHADERS_SOURCE_DIR}"
-DDRECO_SHADERS_BINARY_DIR="${DRECO_SHADERS_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/platforms")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/platforms/paths")

target_link_libraries(${PROJECT_NAME} PUBLIC dreco-sdl)