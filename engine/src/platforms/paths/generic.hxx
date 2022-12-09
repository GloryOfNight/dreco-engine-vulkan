#pragma once

#include <string>

namespace de::paths
{
	struct generic
	{
		static bool init();

		static std::string currentDir();

		static std::string assetsDir();

		static std::string shadersBinDir();

		static std::string shadersSrcDir();

	private:
		static bool checkCorePaths();
	};

} // namespace de::paths