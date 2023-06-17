#pragma once
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class renderer;
	struct settings
	{
		void init();

		vk::PresentModeKHR getPresentMode() const;

		vk::SampleCountFlagBits getSampleCount() const;

		bool IsMultisamplingSupported() const;

		vk::PolygonMode getPolygonMode() const;

		settings& setSampleCount(const vk::SampleCountFlagBits sampleCount);
		settings& setPolygonMode(const vk::PolygonMode mode);

		bool operator==(const settings& other) const
		{
			return memcmp(this, &other, sizeof(settings)) == 0;
		}

	private:
		vk::SampleCountFlagBits _sampleCount{vk::SampleCountFlagBits::e1};

		vk::PolygonMode _polygonMode{vk::PolygonMode::eFill};
	};
} // namespace de::vulkan