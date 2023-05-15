# TODO: 
# Dreco SDL-CXX abstruction

project(dreco-sdl-common)

add_library(${PROJECT_NAME} STATIC include/sdl/sdl.h)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    EXPORT_COMPILE_COMMANDS TRUE
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

find_package(SDL3 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)