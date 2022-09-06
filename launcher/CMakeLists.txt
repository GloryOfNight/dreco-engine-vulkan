cmake_minimum_required(VERSION 3.21.0)
project(dreco-launcher)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco-game dreco-engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>" COMMAND_EXPAND_LISTS)

install(TARGETS dreco-launcher dreco-engine dreco-game
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE EXCLUDE_FROM_ALL)
