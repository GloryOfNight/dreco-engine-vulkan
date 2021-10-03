#include "image_data.hxx"

#include "core/utils/log.hxx"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

image_data::image_data()
	: _pixels{nullptr}
	, _texWidth{0}
	, _texHeight{0}
	, _texChannels{0}
{
}

void image_data::getData(uint8_t** pixels, uint16_t* texWidth, uint16_t* texHeight, uint8_t* texChannels) const
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

image_data::~image_data()
{
	if (_pixels)
	{
		stbi_image_free(_pixels);
	}
}

bool image_data::isLoaded() const
{
	return nullptr != _pixels;
}

bool image_data::load(const std::string_view& texUri)
{
	int w, h, c;
	_pixels = reinterpret_cast<uint8_t*>(stbi_load(texUri.data(), &w, &h, &c, STBI_rgb_alpha));
	_texWidth = static_cast<uint16_t>(w);
	_texHeight = static_cast<uint16_t>(h);
	_texChannels = static_cast<uint16_t>(c);

	return nullptr != _pixels;
}