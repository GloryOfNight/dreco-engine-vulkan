#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

class file_utils
{
public:
	static std::string read_file(const char* path)
	{
		std::string buff;
		std::ifstream file{path, std::ifstream::binary | std::ifstream::ate};

		if (file.is_open())
		{
			const size_t len{static_cast<size_t>(file.tellg())};
			file.seekg(0, file.beg);
			buff.reserve(len);

			buff.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}
		else
		{
			std::cerr << "File not found: " << path << "\n";
		}

		file.close();

		return buff;
	}
};
