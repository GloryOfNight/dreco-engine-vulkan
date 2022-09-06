cmake_minimum_required(VERSION 3.16.2)
project(dreco-game)

set(SOURCES_LIST game.cxx)

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco-engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE)