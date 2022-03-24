#include "generic.hxx"

#include "dreco.hxx"

#include <filesystem>

using namespace std::filesystem;

bool generic_paths::init()
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

	// set back non-modifed path
	current_path(originalCwd);
	return false;
}

std::string generic_paths::currentDir()
{
	return current_path().generic_string();
}

std::string generic_paths::assetsDir()
{
	return (current_path() / DRECO_ASSETS_DIR).generic_string();
}

std::string generic_paths::shadersBinDir()
{
	return (current_path() / DRECO_SHADERS_BINARY_DIR).generic_string();
}

std::string generic_paths::shadersSrcDir()
{
	return (current_path() / DRECO_SHADERS_SOURCE_DIR).generic_string();
}

bool generic_paths::checkCorePaths()
{
	return is_directory(assetsDir()) && (is_directory(shadersBinDir()) || is_directory(shadersSrcDir()));
}
