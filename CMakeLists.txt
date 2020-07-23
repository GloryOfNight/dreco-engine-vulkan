cmake_minimum_required(VERSION 3.7)
project(dreco)

set(DRECO_SOURCE_LIST 
src/math/vec3.cxx
src/math/mat4.cxx
src/render/vulkan/vk_swapchain.cxx
src/render/vulkan/vk_renderer.cxx
src/engine/engine.cxx)

add_library(${PROJECT_NAME} SHARED ${DRECO_SOURCE_LIST})

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (UNIX)
target_link_libraries(${PROJECT_NAME} vulkan)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(MSVC)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DDRECO_DECLSPEC=__declspec(dllexport)")
target_link_libraries(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++17")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/src/)

if (NOT DEFINED DRECO_INSTALL_DIR)
set(DRECO_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/bin)
message("-DDRECO_INSTALL_DIR not set. Setting it for you: " ${DRECO_INSTALL_DIR})
endif()

add_subdirectory(examples/triangle)

install(TARGETS dreco triangle
        RUNTIME DESTINATION ${DRECO_INSTALL_DIR}
        LIBRARY DESTINATION ${DRECO_INSTALL_DIR}
        ARCHIVE DESTINATION ${DRECO_INSTALL_DIR})

if (MSVC)
install(FILES ${SDL2_DLL} DESTINATION ${DRECO_INSTALL_DIR})
endif()