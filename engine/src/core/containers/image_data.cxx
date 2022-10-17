#include "image_data.hxx"

#include "core/utils/log.hxx"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void image_data::getData(uint8_t** pixels, uint16_t* texWidth, uint16_t* texHeight, uint8_t* texChannels) const
{
	if (pixels)
		*pixels = _pixels.get();

	if (texWidth)
		*texWidth = _texWidth;

	if (texHeight)
		*texHeight = _texHeight;

	if (texChannels)
		*texChannels = _texChannels;
}

image_data image_data::makePlaceholder(uint16_t width, uint16_t height, uint8_t channels)
{
	const uint8_t pink[4] = {255, 0, 255, 255};
	const uint8_t black[4] = {0, 0, 0, 255};

	const auto wh = width * height;

	image_data outData;
	outData._texWidth = width;
	outData._texHeight = height;
	outData._texChannels = channels;
	outData._pixels.reset(new uint8_t[wh * channels]);

	bool usePink = false;
	for (uint32_t i = 0; i < wh; ++i)
	{
		auto dst = outData._pixels.get();
		const auto src = i % 3 || i == 1 ? &pink[0] : &black[0];
		memcpy(&dst[i * channels], src, channels);
	}

	return outData;
}

bool image_data::isLoaded() const
{
	return nullptr != _pixels;
}

bool image_data::load(const std::string_view texUri)
{
	int w, h, c;
	const auto stbiPixels = stbi_load(texUri.data(), &w, &h, &c, STBI_rgb_alpha);

	_texWidth = static_cast<uint16_t>(w);
	_texHeight = static_cast<uint16_t>(h);
	_texChannels = static_cast<uint16_t>(c);
	_pixels.reset(reinterpret_cast<uint8_t*>(stbiPixels));

	return nullptr != _pixels;
}