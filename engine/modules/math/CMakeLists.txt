# Dreco own math library
# Written using plain C++ math, in future MUST use SSE and AVX instruction sets!

project(dreco-math)


file(GLOB_RECURSE DRECO_MATH_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx)
file(GLOB_RECURSE DRECO_MATH_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

add_library(${PROJECT_NAME} STATIC ${DRECO_MATH_SOURCE_FILES} ${DRECO_MATH_HEADER_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    EXPORT_COMPILE_COMMANDS TRUE
    POSITION_INDEPENDENT_CODE TRUE
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/math")

# target_compile_options(${PROJECT_NAME} PRIVATE "-mavx" "-mavx2" "-msse" "-msse2" "-msse3" "-mssse3")


### Optional GLM ###
find_package(glm QUIET)
if (${glm_FOUND})
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${glm_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC DRECO_HAS_GLM)
endif()