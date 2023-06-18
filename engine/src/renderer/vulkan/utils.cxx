#include "utils.hxx"

#include <algorithm>

vk::Format de::vulkan::utils::findSupportedDepthFormat(const vk::PhysicalDevice physicalDevice) noexcept
{
	constexpr auto formatCandidates = std::array{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	const auto imageTiling = vk::ImageTiling::eOptimal;
	const auto formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

	for (const auto format : formatCandidates)
	{
		const auto formatProperties = physicalDevice.getFormatProperties(format);
		if (imageTiling == vk::ImageTiling::eLinear &&
			(formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
		}
		else if (imageTiling == vk::ImageTiling::eOptimal &&
				 (formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
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

vk::SurfaceFormatKHR de::vulkan::utils::findSurfaceFormat(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface, const vk::Format format)
{
	const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	const auto pred = [format](const vk::SurfaceFormatKHR surfaceFormat)
	{
		return surfaceFormat.format == format;
	};
	const auto result = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), pred);
	return result != surfaceFormats.end() ? *result : vk::SurfaceFormatKHR();
}

vk::PresentModeKHR de::vulkan::utils::findPresentMode(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	// clang-format off
	constexpr auto modesPriority =	
		std::array
		{
			vk::PresentModeKHR::eMailbox,
			vk::PresentModeKHR::eFifoRelaxed,
			vk::PresentModeKHR::eFifo
		};
	// clang-format on
	const auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
	const auto result = std::find_first_of(modesPriority.begin(), modesPriority.end(), availableModes.begin(), availableModes.end());
	return result != modesPriority.end() ? *result : vk::PresentModeKHR();
}