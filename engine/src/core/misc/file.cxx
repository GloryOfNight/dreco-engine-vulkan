#include "file.hxx"

#include "log.hxx"

#include <filesystem>
#include <fstream>

std::string de::file::read(const std::string_view path)
{
	if (std::filesystem::is_regular_file(path))
	{
		std::ifstream file(path.data(), std::ifstream::binary);
		if (file.is_open())
		{
			const auto fileSize = std::filesystem::file_size(path);
			std::string fileContent = std::string(fileSize, '\0');
			file.read(fileContent.data(), fileContent.size());
			file.close();
			DE_LOG(Verbose, "%s: read %i bytes from: %s", __FUNCTION__, fileContent.size(), path.data());
			return fileContent;
		}
		else
		{
			DE_LOG(Error, "%s: failed to open file: %s", __FUNCTION__, path.data());
		}
	}
	else
	{
		DE_LOG(Error, "%s: not a file: %s", __FUNCTION__, path.data());
	}
	return std::string();
}
