add_subdirectory(shader_list_generator)

set_target_properties(shader_list_generator PROPERTIES 
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tools")