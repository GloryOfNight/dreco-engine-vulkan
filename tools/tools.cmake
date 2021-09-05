# compile tools
execute_process(COMMAND "cmake" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/bin/tools" "-S tools/shader_list_generator" "-B tools/build")
execute_process(COMMAND "cmake" "--build" "tools/build" "--config" "Release")


# execute shader_list_generator and compile shaders
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build/generated/")
set(GENERATED_SHADER_LIST_PATH "${CMAKE_SOURCE_DIR}/build/generated/shader_list.cmake")

set(SHADER_LIST_GENERATOR_EXECUTABLE "${CMAKE_SOURCE_DIR}/bin/tools/shader_list_generator")

execute_process(COMMAND 
"${SHADER_LIST_GENERATOR_EXECUTABLE}"
"${CMAKE_SOURCE_DIR}/shaders"
"${GENERATED_SHADER_LIST_PATH}"
)
include(${GENERATED_SHADER_LIST_PATH})

foreach(shader_path shader_name IN ZIP_LISTS SHADER_COMPILE_PATH_LIST SHADER_COMPILE_NAMES_LIST)
set (SHADER_SOURCE_FILE "${shader_path}/${shader_name}")
set (SHADER_OUTPUT_FILE "${shader_path}/${shader_name}.spv")
message(STATUS "Compiling shader: ${SHADER_SOURCE_FILE} -> ${SHADER_OUTPUT_FILE}")
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${SHADER_SOURCE_FILE}" "-o" "${SHADER_OUTPUT_FILE}")
endforeach()