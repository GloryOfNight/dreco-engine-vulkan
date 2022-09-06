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

image_data image_data::createPlaceholderTexture()
{
	constexpr uint16_t texSize = 128;
	constexpr uint32_t texSizeX2 = texSize * texSize;
	constexpr uint8_t texChannels = 4;

	const uint8_t pink[4] = {255, 0, 255, 255};
	const uint8_t black[4] = {0, 0, 0, 255};

	image_data outData;
	outData._texWidth = texSize;
	outData._texHeight = texSize;
	outData._texChannels = texChannels;
	outData._pixels = new uint8_t[texSizeX2 * texChannels];

	bool usePink = false;
	for (uint32_t i = 0; i < texSizeX2; ++i)
	{
		const auto* src = i % 3 || i == 1 ? &pink[0] : &black[0];
		memcpy(&outData._pixels[i * texChannels], src, texChannels);
	}

	return outData;
}

image_data::image_data(image_data&& other)
{
	_texWidth = other._texWidth;
	_texHeight = other._texHeight;
	_texChannels = other._texChannels;
	_pixels = other._pixels;

	other._texWidth = other._texHeight = other._texChannels = 0;
	other._pixels = nullptr;
}

image_data::~image_data()
{
	if (_pixels)
	{
		stbi_image_free(_pixels);
		_pixels = nullptr;
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