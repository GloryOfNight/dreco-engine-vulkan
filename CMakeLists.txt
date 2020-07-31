cmake_minimum_required(VERSION 3.16.2)
project(dreco)

set(DRECO_SOURCE_LIST 
src/engine/engine.cxx
src/math/vec3.cxx
src/math/mat4.cxx
src/renderer/vulkan/vk_swapchain.cxx
src/renderer/vulkan/vk_renderer.cxx
src/renderer/vulkan/vk_mesh.cxx)

add_library(${PROJECT_NAME} SHARED ${DRECO_SOURCE_LIST})

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (UNIX)
target_link_libraries(${PROJECT_NAME} PRIVATE vulkan)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
elseif(MSVC)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DDRECO_DECLSPEC=__declspec(dllexport)")
target_link_libraries(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++17")
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