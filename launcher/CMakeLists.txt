cmake_minimum_required(VERSION 3.21.0)
project(dreco-launcher)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco-game dreco-engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    EXPORT_COMPILE_COMMANDS TRUE
)

install(TARGETS dreco-launcher dreco-engine dreco-game
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE EXCLUDE_FROM_ALL)

if(UNIX)
    find_library(TBB_LIB NAMES tbb REQUIRED)
    target_link_libraries(${PROJECT_NAME} "tbb")
endif()