#pragma once
#include "log.hxx"

#include <filesystem>
#include <fstream>
#include <string>

class file_utils
{
public:
	template<typename Str>
	static std::string readFile(Str&& path)
	{
		if (std::filesystem::is_regular_file(path))
		{
			std::ifstream file(path, std::ifstream::binary);
			if (file.is_open())
			{
				const auto fileSize = std::filesystem::file_size(path);
				std::string fileContent = std::string(fileSize, '\0');
				file.read(fileContent.data(), fileContent.size());
				file.close();
				return fileContent;
			}
			else
			{
				DE_LOG(Error, "%s: Failed to open file: %s", __FUNCTION__, path);
			}
		}
		else
		{
			DE_LOG(Error, "%s: Not a file: %s", __FUNCTION__, path);
		}
		return std::string();
	}
};
