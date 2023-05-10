cmake_minimum_required(VERSION 3.16.2)
project(dreco-engine)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm QUIET)

set(DRECO_ASSETS_DIR "assets")
set(DRECO_SHADERS_SOURCE_DIR "shaders/src")
set(DRECO_SHADERS_BINARY_DIR "shaders/bin")
set(DRECO_LIBRARY_TYPE SHARED)

file(GLOB_RECURSE DRECO_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE DRECO_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)

add_library(${PROJECT_NAME} ${DRECO_LIBRARY_TYPE} ${DRECO_SOURCE_FILES} ${DRECO_HEADER_FILES})

if (${DRECO_LIBRARY_TYPE} STREQUAL SHARED)
target_compile_definitions(${PROJECT_NAME} PRIVATE DRECO_LIBRARY_TYPE_SHARED)
elseif(${DRECO_LIBRARY_TYPE} STREQUAL STATIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE DRECO_LIBRARY_TYPE_STATIC)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_VK_USE_VALIDATION DRECO_VK_USE_MESA_OVERLAY DRECO_VK_USE_LUNAR_MONITOR)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan SDL2::SDL2 shader-compiler-static spirv-reflect stb tinygltf)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

if (${glm_FOUND})
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${glm_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_HAS_GLM)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    EXPORT_COMPILE_COMMANDS TRUE
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
-DDRECO_ASSETS_DIR="${DRECO_ASSETS_DIR}"
-DDRECO_SHADERS_SOURCE_DIR="${DRECO_SHADERS_SOURCE_DIR}"
-DDRECO_SHADERS_BINARY_DIR="${DRECO_SHADERS_BINARY_DIR}")

add_subdirectory(libs)
target_link_libraries(${PROJECT_NAME} PUBLIC dreco-math)

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