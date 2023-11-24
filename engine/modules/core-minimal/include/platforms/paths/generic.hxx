#pragma once

#include <string>

namespace de::paths
{
	struct generic
	{
		[[nodiscard]] static bool init();

		[[nodiscard]] static std::string currentDir();

		[[nodiscard]] static std::string assetsDir();

		[[nodiscard]] static std::string shadersBinDir();

		[[nodiscard]] static std::string shadersSrcDir();
	};

} // namespace de::paths