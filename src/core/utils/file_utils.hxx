#pragma once
#include <fstream>
#include <iostream>
#include <stdint.h>

class file_utils
{
public:
	static char* read_file(const char* path, size_t* size)
	{
		char* buff{nullptr};
		std::ifstream file{path, std::ifstream::binary | std::ifstream::ate};

		if (file.is_open())
		{
			const size_t len{static_cast<size_t>(file.tellg())};
			file.seekg(0, file.beg);
			buff = new char[len];

			file.read(buff, len);
			//buff[len + 1] = '\0';
			if (size)
			{
				*size = len;
			}
		}
		else
		{
			std::cerr << "File not found: " << path << "\n";
		}

		file.close();

		return buff;
	}
};
