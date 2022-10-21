project(spirv-reflect)

find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} STATIC "src/spirv_reflect.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/spirv-reflect")
target_compile_definitions(${PROJECT_NAME} PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    EXPORT_COMPILE_COMMANDS TRUE
    POSITION_INDEPENDENT_CODE TRUE
)