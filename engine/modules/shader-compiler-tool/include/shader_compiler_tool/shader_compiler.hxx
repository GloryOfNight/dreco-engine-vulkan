#pragma once

#include <string_view>

struct shader_compiler
{
	static int attemptCompileShaders(const std::string_view& srcShaderDir, const std::string_view& binShaderDir);
};