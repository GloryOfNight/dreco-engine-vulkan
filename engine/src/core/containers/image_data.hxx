#pragma once
#include <string_view>
#include <vector>

namespace de
{
	struct image_data
	{
		static image_data load(const std::string_view texUri, const uint8_t Components = 4);
		static image_data makePlaceholder(uint16_t width = 128, uint16_t heigth = 128, uint8_t components = 4);

		bool isValid() const;

		uint16_t getWidth() const;
		uint16_t getHeight() const;
		uint8_t getChannels() const;
		uint8_t getComponents() const;

		size_t getPixelCount() const;
		uint8_t const* getPixels() const;

	protected:
		image_data& setWidth(const uint16_t inValue);
		image_data& setHeight(const uint16_t inValue);
		image_data& setChannels(const uint8_t inValue);
		image_data& setComponents(const uint8_t inValue);
		image_data& setPixelCount(const size_t inValue);
		image_data& setPixels(uint8_t* inValue, size_t size);

	private:
		uint16_t _width{};
		uint16_t _height{};
		uint8_t _channels{};
		uint8_t _components{};
		std::vector<uint8_t> _pixels;
	};
} // namespace de