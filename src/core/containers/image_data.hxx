#pragma once
#include <string_view>

struct image_data
{
	image_data();
	image_data(const image_data&) = delete;
	image_data(image_data&&) = default;
	~image_data();

	bool isLoaded() const;
	bool load(const std::string_view& texUri);

	void getData(uint8_t** pixels, uint16_t* texWidth, uint16_t* texHeight, uint8_t* texChannels) const;

protected:
	uint8_t* _pixels;
	uint16_t _texWidth;
	uint16_t _texHeight;
	uint8_t _texChannels;
};