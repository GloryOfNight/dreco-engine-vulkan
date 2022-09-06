cmake_minimum_required(VERSION 3.16.2)
project(dreco-game)

set(SOURCES_LIST game.cxx)

if(DRECO_BUILD_SHARED)
add_library(${PROJECT_NAME} SHARED ${SOURCES_LIST})
else()
add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})
endif()

target_link_libraries(${PROJECT_NAME} dreco-engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE)