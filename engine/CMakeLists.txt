cmake_minimum_required(VERSION 3.16.2)
project(dreco-engine)

find_package(Vulkan)
if(NOT DEFINED Vulkan_FOUND)
    message(FATAL_ERROR "Failed to find Vulkan package")
endif()

find_package(SDL2)
if(NOT DEFINED SDL2_FOUND)
    message(FATAL_ERROR "Failed to find SDL2 package")
endif()

set(DRECO_ASSETS_DIR "assets")
set(DRECO_SHADERS_SOURCE_DIR "shaders/src")
set(DRECO_SHADERS_BINARY_DIR "shaders/bin")

file(GLOB_RECURSE DRECO_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE DRECO_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)

if (DRECO_BUILD_SHARED)
add_library(${PROJECT_NAME} SHARED ${DRECO_SOURCE_FILES} ${DRECO_HEADER_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_BUILD_SHARED)
else()
add_library(${PROJECT_NAME} STATIC ${DRECO_SOURCE_FILES} ${DRECO_HEADER_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_BUILD_STATIC)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 Vulkan::Vulkan shader-compiler-static spirv-reflect)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    EXPORT_COMPILE_COMMANDS TRUE
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
-DDRECO_ASSETS_DIR="${DRECO_ASSETS_DIR}"
-DDRECO_SHADERS_SOURCE_DIR="${DRECO_SHADERS_SOURCE_DIR}"
-DDRECO_SHADERS_BINARY_DIR="${DRECO_SHADERS_BINARY_DIR}")

add_subdirectory(tools)
add_subdirectory(thirdparty)

if(WIN32)
add_custom_command(
  TARGET  ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
endif()

install(IMPORTED_RUNTIME_ARTIFACTS SDL2::SDL2 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})