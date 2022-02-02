#pragma once

#include <string_view>

#ifndef DRECO_SHADER_COMPILER_API
#ifndef BUILD_STATIC
#define DRECO_SHADER_COMPILER_API
#else
#if _WIN32
#define DRECO_SHADER_COMPILER_API __declspec(dllexport)
#endif
#endif
#endif

struct DRECO_SHADER_COMPILER_API shader_compiler
{
	static int attemptCompileShaders(const std::string_view& srcShaderDir, const std::string_view& binShaderDir);
};