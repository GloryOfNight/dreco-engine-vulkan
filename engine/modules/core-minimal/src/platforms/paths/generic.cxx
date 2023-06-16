#include "generic.hxx"

#include "dreco.hxx"

#include <filesystem>

using namespace std::filesystem;

bool de::paths::generic::init()
{
	if (checkCorePaths())
	{
		return true;
	}

	const auto originalCwd = current_path();
	// check lower 2 times. .
	for (uint8_t i = 0; i < 2; i++)
	{
		current_path(current_path() / "..");
		if (checkCorePaths())
		{
			return true;
		}
	}

	// set back non-modified path
	current_path(originalCwd);
	return false;
}

std::string de::paths::generic::currentDir()
{
	return current_path().generic_string();
}

std::string de::paths::generic::assetsDir()
{
	return (current_path() / DRECO_ASSETS_DIR).generic_string();
}

std::string de::paths::generic::shadersBinDir()
{
	return (current_path() / DRECO_SHADERS_BINARY_DIR).generic_string();
}

std::string de::paths::generic::shadersSrcDir()
{
	return (current_path() / DRECO_SHADERS_SOURCE_DIR).generic_string();
}

bool de::paths::generic::checkCorePaths()
{
	return is_directory(assetsDir()) && (is_directory(shadersBinDir()) || is_directory(shadersSrcDir()));
}
