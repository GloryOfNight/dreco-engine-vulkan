# "Game" library that uses a engine library to create a game
project(dreco-game)

file(GLOB_RECURSE GAME_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE GAME_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)

add_library(${PROJECT_NAME} OBJECT ${GAME_SOURCE_FILES} ${GAME_HEADER_FILES})

add_library(${PROJECT_NAME}-shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}-shared ${PROJECT_NAME} dreco-engine-shared)

add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}-static ${PROJECT_NAME} dreco-engine-static)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE
)
target_link_libraries(${PROJECT_NAME} dreco-engine)

install(DIRECTORY ${DRECO_ASSETS_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${DRECO_SHADERS_BINARY_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)