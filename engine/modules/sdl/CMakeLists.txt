# TODO: 
# Dreco SDL-CXX abstruction

project(dreco-sdl)

add_library(${PROJECT_NAME} STATIC "src/sdl.cxx")
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

find_package(SDL3 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3-shared)

if(WIN32)
add_custom_command(
    TARGET  ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:SDL3::SDL3-shared>" "$<TARGET_FILE_DIR:dreco-engine-shared>"
)
endif()

install(IMPORTED_RUNTIME_ARTIFACTS SDL3::SDL3-shared RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})