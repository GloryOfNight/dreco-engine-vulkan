#pragma once
#include <string_view>

struct texture_data
{
	static texture_data* createNew(const std::string_view& texUri);

	texture_data(const texture_data&) = delete;
	texture_data(texture_data&&) = delete;
	~texture_data();

	void getData(unsigned char** pixels, int* texWidth, int* texHeight, int* texChannels) const;

protected:
	texture_data(const std::string_view& texUri);

	unsigned char* _pixels;
	int _texWidth;
	int _texHeight;
	int _texChannels;
};