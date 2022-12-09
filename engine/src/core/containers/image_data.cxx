#include "image_data.hxx"

#include "dreco.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

uint16_t de::image_data::getWidth() const
{
	return _width;
}

uint16_t de::image_data::getHeight() const
{
	return _height;
}

uint8_t de::image_data::getChannels() const
{
	return _channels;
}

uint8_t de::image_data::getComponents() const
{
	return _components;
}

uint8_t const* de::image_data::getPixels() const
{
	return _pixels.data();
}

size_t de::image_data::getPixelCount() const
{
	return _pixels.size();
}

de::image_data& de::image_data::setWidth(const uint16_t inValue)
{
	_width = inValue;
	return *this;
};

de::image_data& de::image_data::setHeight(const uint16_t inValue)
{
	_height = inValue;
	return *this;
};

de::image_data& de::image_data::setChannels(const uint8_t inValue)
{
	_channels = inValue;
	return *this;
};

de::image_data& de::image_data::setComponents(const uint8_t inValue)
{
	_components = inValue;
	return *this;
};

de::image_data& de::image_data::setPixelCount(const size_t inValue)
{
	_pixels.resize(inValue);
	return *this;
};

de::image_data& de::image_data::setPixels(uint8_t* inValue, size_t size)
{
	if (_pixels.size() != size)
		_pixels.resize(size);

	memmove(_pixels.data(), inValue, size);
	return *this;
};

de::image_data de::image_data::makePlaceholder(uint16_t width, uint16_t heigth, uint8_t components)
{
	const uint8_t pink[4] = {255, 0, 255, 255};
	const uint8_t black[4] = {0, 0, 0, 255};

	const auto wh = width * heigth;
	image_data outData = image_data()
							 .setWidth(width)
							 .setHeight(heigth)
							 .setChannels(3)
							 .setComponents(components)
							 .setPixelCount(wh * components);

	for (uint32_t i = 0; i < wh; ++i)
	{
		const auto src = i % 3 || i == 1 ? &pink[0] : &black[0];
		memcpy(&outData._pixels[i * components], src, sizeof(pink));
	}

	return outData;
}

bool de::image_data::isValid() const
{
	return getWidth() > 0 && getHeight() > 0 && getChannels() > 0 && getComponents() > 0 && getPixelCount() > 0;
}

de::image_data de::image_data::load(const std::string_view texUri, const uint8_t components)
{
	int width, heigth, channels;
	const auto stbiPixels = stbi_load(texUri.data(), &width, &heigth, &channels, components);

	if (stbiPixels)
	{
		const size_t pixelCount = width * heigth * components;
		image_data outData = image_data()
								 .setWidth(width)
								 .setHeight(heigth)
								 .setChannels(channels)
								 .setComponents(components)
								 .setPixelCount(pixelCount)
								 .setPixels(stbiPixels, pixelCount);
		delete[] stbiPixels;
		return outData;
	}

	DE_LOG(Error, "%s: Failed to load image: %s, using placeholder.", __FUNCTION__, texUri.data());
	return image_data::makePlaceholder(256, 256, components);
}