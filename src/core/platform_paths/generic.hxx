#pragma once

#include <string>

struct generic_paths
{
	static bool init();

	static std::string currentDir();

	static std::string assetsDir();

	static std::string shadersDir();

private:
	static bool checkCorePaths();
};