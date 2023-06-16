# Dreco gltf library that abstructs over tinygltf library
project(dreco-gltf)

file(GLOB_RECURSE DRECO_GLTF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx)
file(GLOB_RECURSE DRECO_GLTF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

add_library(${PROJECT_NAME} STATIC ${DRECO_GLTF_HEADER_FILES} ${DRECO_GLTF_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/gltf")

target_link_libraries(${PROJECT_NAME} PUBLIC tinygltf stb dreco-core-minimal dreco-math)