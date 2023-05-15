# Engine itself
project(dreco-engine)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

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

# link enxternal libraries
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan SDL3::SDL3 spirv-reflect stb tinygltf)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR} ${SDL3_INCLUDE_DIRS})

# link modules
add_subdirectory(modules)
target_link_libraries(${PROJECT_NAME} PUBLIC dreco-math dreco-core-minimal dreco-threads dreco-shader-compiler-tool)

# link thirdparty libraries
add_subdirectory(thirdparty)
target_link_libraries(${PROJECT_NAME} PUBLIC spirv-reflect stb tinygltf)


set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    EXPORT_COMPILE_COMMANDS TRUE
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

if(WIN32)
add_custom_command(
      TARGET  ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
endif()

# TODO: for some reason, this doesn't work. Fix later
#install(IMPORTED_RUNTIME_ARTIFACTS SDL3::SDL3 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})