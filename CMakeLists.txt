cmake_minimum_required(VERSION 3.16.2)
project(dreco)

set(DRECO_SOURCE_LIST 
"src/engine/engine.cxx"
"src/core/threads/thread_pool.cxx"
"src/core/threads/thread_pool.hxx"
"src/core/loaders/gltf_loader.cxx"
"src/core/loaders/gltf_loader.hxx"
"src/core/objects/camera.cxx"
"src/core/objects/camera.hxx"
"src/core/objects/world_object.cxx"
"src/core/objects/world_object.hxx"
"src/math/vec2.cxx"
"src/math/vec3.cxx"
"src/math/rotator.cxx"
"src/math/mat4.cxx"
"src/math/transform.cxx"
"src/renderer/containers/vertex.cxx"
"src/renderer/containers/texture_data.cxx"
"src/renderer/vulkan/vk_surface.cxx"
"src/renderer/vulkan/vk_physical_device.cxx"
"src/renderer/vulkan/vk_renderer.cxx"
"src/renderer/vulkan/vk_mesh.cxx"
"src/renderer/vulkan/vk_device.cxx"
"src/renderer/vulkan/vk_queue_family.cxx"
"src/renderer/vulkan/vk_device_memory.cxx"
"src/renderer/vulkan/vk_buffer.cxx"
"src/renderer/vulkan/vk_shader_module.cxx"
"src/renderer/vulkan/vk_swapchain.cxx"
"src/renderer/vulkan/vk_descriptor_set.cxx"
"src/renderer/vulkan/vk_graphics_pipeline.cxx"
"src/renderer/vulkan/vk_image.cxx"
"src/renderer/vulkan/vk_texture_image.cxx"
"src/renderer/vulkan/vk_msaa_image.cxx"
"src/renderer/vulkan/vk_depth_image.cxx")

add_library(${PROJECT_NAME} SHARED ${DRECO_SOURCE_LIST})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARY})
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("thirdparty")

if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
elseif(MSVC)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DDRECO_DECLSPEC=__declspec(dllexport)" "-D_USE_MATH_DEFINES=true")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if (NOT DEFINED CMAKE_INSTALL_RPATH)
set(CMAKE_INSTALL_RPATH ${CMAKE_CURRENT_LIST_DIR}/bin)
message("-DCMAKE_INSTALL_RPATH not set. Setting it for you: " ${CMAKE_INSTALL_RPATH})
endif()

add_subdirectory(examples/triangle)

install(TARGETS dreco triangle
        RUNTIME DESTINATION ${CMAKE_INSTALL_RPATH}
        LIBRARY DESTINATION ${CMAKE_INSTALL_RPATH}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_RPATH})

if (MSVC)
install(FILES ${SDL2_DLL} DESTINATION ${CMAKE_INSTALL_RPATH})
endif()