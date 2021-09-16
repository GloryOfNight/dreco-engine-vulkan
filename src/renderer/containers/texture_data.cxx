#include "texture_data.hxx"

#include "core/utils/log.hxx"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

texture_data* texture_data::createNew(const std::string_view& texUri)
{
	return new texture_data(texUri);
}

texture_data::texture_data(const std::string_view& texUri)
	: _pixels{nullptr}
	, _texWidth{0}
	, _texHeight{0}
	, _texChannels{0}
{
	_pixels = stbi_load(texUri.data(), &_texWidth, &_texHeight, &_texChannels, STBI_rgb_alpha);
	if (!_pixels)
	{
		DE_LOG(Error, "Failed to load texture from uri: %s", texUri.data());
	}
}

void texture_data::getData(unsigned char** pixels, int* texWidth, int* texHeight, int* texChannels) const
{
	if (pixels)
		*pixels = _pixels;

	if (texWidth)
		*texWidth = _texWidth;

	if (texHeight)
		*texHeight = _texHeight;

	if (texChannels)
		*texChannels = _texChannels;
}

texture_data::~texture_data()
{
	if (_pixels)
	{
		stbi_image_free(_pixels);
	}
}