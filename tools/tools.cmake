if (MSVC)
set(TOOLS_CMAKE_RUNTIME_OUTPUT_DIRECTORY_PARAM "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE")
else()
set(TOOLS_CMAKE_RUNTIME_OUTPUT_DIRECTORY_PARAM "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY")
endif()

# configure and compile tools
execute_process(COMMAND "cmake" 
"${TOOLS_CMAKE_RUNTIME_OUTPUT_DIRECTORY_PARAM}=${CMAKE_SOURCE_DIR}/bin/tools" 
"-S" "${CMAKE_SOURCE_DIR}/tools/shader_list_generator" 
"-B" "${CMAKE_SOURCE_DIR}/tools/build")
execute_process(COMMAND "cmake" "--build" "${CMAKE_SOURCE_DIR}/tools/build" "--config" "Release")


# everytime configuration happend, gather all shaders files and recompile required ones
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build/generated/")
set(GENERATED_SHADER_LIST_PATH "${CMAKE_SOURCE_DIR}/build/generated/shader_list.cmake")

set(SHADER_LIST_GENERATOR_EXECUTABLE "${CMAKE_SOURCE_DIR}/bin/tools/shader_list_generator")

execute_process(COMMAND 
"${SHADER_LIST_GENERATOR_EXECUTABLE}"
"${CMAKE_SOURCE_DIR}/${DRECO_SHADERS_SOURCE_DIR}"
"${CMAKE_SOURCE_DIR}/${DRECO_SHADERS_BINARY_DIR}"
"${GENERATED_SHADER_LIST_PATH}"
)
include(${GENERATED_SHADER_LIST_PATH})

foreach(shader_path shader_name IN ZIP_LISTS SHADER_COMPILE_PATH_LIST SHADER_COMPILE_NAMES_LIST)
set (SHADER_SOURCE_FILE "${shader_path}/${shader_name}")
set (SHADER_OUTPUT_FILE "${CMAKE_SOURCE_DIR}/${DRECO_SHADERS_BINARY_DIR}/${shader_name}.spv")
message(STATUS "Compiling shader: ${SHADER_SOURCE_FILE} -> ${SHADER_OUTPUT_FILE}")
execute_process(COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${SHADER_SOURCE_FILE}" "-o" "${SHADER_OUTPUT_FILE}")
endforeach()