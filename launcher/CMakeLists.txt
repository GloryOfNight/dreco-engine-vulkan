cmake_minimum_required(VERSION 3.16.2)
project(dreco-launcher)

set(SOURCES_LIST main.cxx)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_link_libraries(${PROJECT_NAME} dreco-game)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE)

if (WIN32)
find_package(SDL2 REQUIRED)

get_target_property(SDL2_DLL_LOC SDL2::SDL2 LOCATION_${CMAKE_BUILD_TYPE})

file(COPY ${SDL2_DLL_LOC} DESTINATION ${PROJECT_BINARY_DIR})
install(FILES ${SDL2_DLL_LOC} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
