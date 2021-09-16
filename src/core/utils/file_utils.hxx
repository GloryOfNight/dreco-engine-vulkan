#pragma once
#include <filesystem>
#include <fstream>
#include <string>

#include "log.hxx"

class file_utils
{
public:
	static bool isFileExists(const std::string_view& path)
	{
		return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
	};

	static std::string currentWorkingDir() 
	{
		return std::filesystem::current_path().string();
	}

	static bool readFile(const std::string_view& path, std::string& data)
	{
		data.clear();
		std::ifstream file{path.data(), std::ifstream::binary | std::ifstream::ate};

		const bool isFileRead = file.is_open();
		if (isFileRead)
		{
			const size_t len{static_cast<size_t>(file.tellg())};
			file.seekg(0, file.beg);
			data.reserve(len);
			data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}
		else
		{
			DE_LOG(Error, "Failed to load file: %s", path.data());
		}

		file.close();

		return isFileRead;
	}
};
