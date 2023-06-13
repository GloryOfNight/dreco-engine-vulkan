#pragma once
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class renderer;
	struct settings
	{
		void init(const renderer* renderer);

		const vk::SurfaceFormatKHR& getSurfaceFormat() const;

		vk::PresentModeKHR getPresentMode() const;

		vk::SampleCountFlagBits getMaxSampleCount() const;

		vk::SampleCountFlagBits getPrefferedSampleCount() const;
		bool setPrefferedSampleCount(const vk::SampleCountFlagBits sampleCount);

		bool getIsSamplingSupported() const;

		vk::PolygonMode getDefaultPolygonMode() const;
		bool setDefaultPolygonMode(const vk::PolygonMode mode);

	private:
		vk::SurfaceFormatKHR _surfaceFormat;
		vk::PresentModeKHR _presentMode;

		vk::SampleCountFlagBits _maxSampleCount{vk::SampleCountFlagBits::e1};

		vk::SampleCountFlagBits _prefferedSampleCount{vk::SampleCountFlagBits::e1};

		vk::PolygonMode _polygonMode{vk::PolygonMode::eFill};
	};
} // namespace de::vulkan