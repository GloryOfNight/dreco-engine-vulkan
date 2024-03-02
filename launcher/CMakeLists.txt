# A executable that glues Game and Engine parts as well as future foundation for other runables (like editor)
project(dreco-launcher)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco-game-shared dreco-engine-shared)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    EXPORT_COMPILE_COMMANDS TRUE
)

install(TARGETS dreco-launcher dreco-engine-shared dreco-game-shared
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE EXCLUDE_FROM_ALL)