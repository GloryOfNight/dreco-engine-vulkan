#include "generic.hxx"

#include "dreco.hxx"

#include <array>
#include <filesystem>

bool de::paths::generic::init()
{
	const std::array paths{
		std::filesystem::current_path(),
		std::filesystem::current_path() / "game",
		std::filesystem::current_path() / ".." / "..",
		std::filesystem::current_path() / ".." / ".." / "game",
	};

	for (const auto& path : paths)
	{
		if (std::filesystem::is_directory(path / DRECO_ASSETS_DIR) && (std::filesystem::is_directory(path / DRECO_SHADERS_BINARY_DIR) || std::filesystem::is_directory(path / DRECO_SHADERS_SOURCE_DIR)))
		{
			std::filesystem::current_path(path);

			DE_LOG(Info, "Found cdw: %s", path.generic_string().data());
			return true;
		}
	}
	return false;
}

std::string de::paths::generic::currentDir()
{
	return std::filesystem::current_path().generic_string();
}

std::string de::paths::generic::assetsDir()
{
	return (std::filesystem::current_path() / DRECO_ASSETS_DIR).generic_string();
}

std::string de::paths::generic::shadersBinDir()
{
	return (std::filesystem::current_path() / DRECO_SHADERS_BINARY_DIR).generic_string();
}

std::string de::paths::generic::shadersSrcDir()
{
	return (std::filesystem::current_path() / DRECO_SHADERS_SOURCE_DIR).generic_string();
}
