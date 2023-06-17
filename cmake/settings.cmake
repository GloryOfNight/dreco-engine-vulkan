
# binaries and libraries should be placed in the same directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Runtime Output Directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "Static Libraries Output Directory")

# static libraries should be build with position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Use Position Independent Code")

# export compile commands for code editors
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export Compile Commands")