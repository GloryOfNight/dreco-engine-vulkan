#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace de::gltf
{
	struct image
	{
		std::string _uri;

		uint16_t _width{};

		uint16_t _height{};

		uint8_t _channels{};

		uint8_t _components{};

		std::vector<uint8_t> _pixels;

		static image makePlaceholder(uint16_t width, uint16_t height, uint8_t channels = 3U, uint8_t components = 4U)
		{
			image outImage = image{
				._width = width,
				._height = height,
				._channels = channels,
				._components = components,
				._pixels = std::vector<uint8_t>(width * height * components, 0)};

			const uint8_t pink[4] = {255, 0, 255, 255};
			const uint8_t black[4] = {0, 0, 0, 255};

			const auto wh = width * height;
			for (uint32_t i = 0; i < wh; ++i)
			{
				const auto src = i % 3 || i == 1 ? &pink[0] : &black[0];
				memcpy(&outImage._pixels[i * components], src, sizeof(pink));
			}

			return outImage;
		}
	};
} // namespace de::gltf
