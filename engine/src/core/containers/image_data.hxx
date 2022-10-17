#pragma once
#include <string_view>
#include <memory>

struct image_data
{
	bool isLoaded() const;
	bool load(const std::string_view texUri);

	void getData(uint8_t** pixels, uint16_t* texWidth, uint16_t* texHeight, uint8_t* texChannels) const;

	static image_data makePlaceholder(uint16_t width = 128, uint16_t height = 128, uint8_t channels = 4);

protected:
	uint16_t _texWidth{};
	uint16_t _texHeight{};
	uint8_t _texChannels{};
	std::unique_ptr<uint8_t> _pixels;
};