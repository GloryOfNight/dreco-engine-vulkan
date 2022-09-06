#pragma once

#include <string>

struct generic_paths
{
	static bool init();

	static std::string currentDir();

	static std::string assetsDir();

	static std::string shadersBinDir();

	static std::string shadersSrcDir();

private:
	static bool checkCorePaths();
};