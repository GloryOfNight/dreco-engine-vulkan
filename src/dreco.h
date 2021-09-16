#pragma once
#include <string>

#ifndef DRECO_DECLSPEC
#define DRECO_DECLSPEC
#endif

#define STRINGFY(value) value

#define DRECO_ASSET(name) (std::string(DRECO_ASSETS_DIR) + "/" + name)
#define DRECO_SHADER(name) (std::string(DRECO_SHADERS_BINARY_DIR) + "/" + name)

#define TEXTURE_PLACEHOLDER_URI DRECO_ASSET("default.jpg")

#define SHADER_BASIC_VERTEX_BIN_URI DRECO_SHADER("basic.vert.spv")
#define SHADER_BASIC_FRAGMENT_BIN_URI DRECO_SHADER("basic.frag.spv")