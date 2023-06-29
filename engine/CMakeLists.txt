# main CMakeLists.txt for engine
project(dreco-engine)

find_package(Vulkan REQUIRED)

set(DRECO_ASSETS_DIR "assets")
set(DRECO_SHADERS_SOURCE_DIR "shaders/src")
set(DRECO_SHADERS_BINARY_DIR "shaders/bin")

file(GLOB_RECURSE DRECO_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE DRECO_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)

# creating object library to be compiled and later linked into shared and static libraries
add_library(${PROJECT_NAME} OBJECT ${DRECO_SOURCE_FILES} ${DRECO_HEADER_FILES})

# add shared library using object library
add_library(${PROJECT_NAME}-shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}-shared ${PROJECT_NAME})

# add static library using object library
add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}-static ${PROJECT_NAME})

## setting up object library and shared/static libraries using it
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# make sure src directory is included to public 
# TODO: separate src and include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

# used by renderer 
# TODO: move somewhere else
target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_VK_USE_VALIDATION DRECO_VK_USE_MESA_OVERLAY DRECO_VK_USE_LUNAR_MONITOR)

# link enxternal libraries
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan spirv-reflect stb tinygltf)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR} ${SDL3_INCLUDE_DIRS})

# link modules
add_subdirectory(modules)
target_link_libraries(${PROJECT_NAME} PUBLIC dreco-core-minimal dreco-math dreco-gltf dreco-threads dreco-shader-compiler-tool)

# link thirdparty libraries
add_subdirectory(thirdparty)
target_link_libraries(${PROJECT_NAME} PUBLIC spirv-reflect stb tinygltf)


