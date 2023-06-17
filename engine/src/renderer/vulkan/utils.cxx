#include "utils.hxx"

vk::Format de::vulkan::utils::findSupportedFormat(const vk::PhysicalDevice physicalDevice, const find_supported_format_info& info) noexcept
{
	if (physicalDevice)
	{
		for (const vk::Format format : info._formatCandidates)
		{
			const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

			if (info._imageTiling == vk::ImageTiling::eLinear &&
				(formatProperties.linearTilingFeatures & info._formatFeatureFlags) == info._formatFeatureFlags)
			{
				return format;
			}
			else if (info._imageTiling == vk::ImageTiling::eOptimal &&
					 (formatProperties.optimalTilingFeatures & info._formatFeatureFlags) == info._formatFeatureFlags)
			{
				return format;
			}
		}
	}
	return vk::Format::eUndefined;
}

vk::SampleCountFlagBits de::vulkan::utils::findMaxSampleCount(const vk::PhysicalDevice physicalDevice)
{
	const auto limits = physicalDevice.getProperties().limits;
	const auto counts = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
	if (counts & vk::SampleCountFlagBits::e64)
		return vk::SampleCountFlagBits::e64;
	else if (counts & vk::SampleCountFlagBits::e32)
		return vk::SampleCountFlagBits::e32;
	else if (counts & vk::SampleCountFlagBits::e16)
		return vk::SampleCountFlagBits::e16;
	else if (counts & vk::SampleCountFlagBits::e8)
		return vk::SampleCountFlagBits::e8;
	else if (counts & vk::SampleCountFlagBits::e4)
		return vk::SampleCountFlagBits::e4;
	else if (counts & vk::SampleCountFlagBits::e2)
		return vk::SampleCountFlagBits::e2;
	return vk::SampleCountFlagBits::e1;
}

vk::SurfaceFormatKHR de::vulkan::utils::findSurfaceFormat(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

	for (const auto& surfaceFormat : surfaceFormats)
	{
		if (vk::Format::eB8G8R8A8Unorm == surfaceFormat.format)
		{
			return surfaceFormat;
		}
	}
	throw std::runtime_error("Failed to find preffered surface format!");
	return vk::SurfaceFormatKHR();
}

vk::PresentModeKHR de::vulkan::utils::findPresentMode(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	// clang-format off
	const std::array<vk::PresentModeKHR, 4> modesPriority = // prefer top ones over lower ones
		{
			vk::PresentModeKHR::eMailbox,
			vk::PresentModeKHR::eFifoRelaxed,
			vk::PresentModeKHR::eFifo,
			vk::PresentModeKHR::eImmediate
		};
	// clang-format on

	const auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
	const auto availableModesBegin = availableModes.begin();
	const auto availableModesEnd = availableModes.end();
	for (const auto mode : modesPriority)
	{
		if (std::find(availableModesBegin, availableModesEnd, mode) != availableModesEnd)
		{
			return mode;
		}
	}
	return vk::PresentModeKHR::eImmediate;
}