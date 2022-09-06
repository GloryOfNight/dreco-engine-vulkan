#include "vk_utils.hxx"

vk::Format vk_utils::findSupportedFormat(const vk::PhysicalDevice physicalDevice, const find_supported_format_info& info) noexcept
{
	if (physicalDevice)
	{
		for (const vk::Format format : info.formatCandidates)
		{
			const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

			if (info.imageTiling == vk::ImageTiling::eLinear &&
				(formatProperties.linearTilingFeatures & info.formatFeatureFlags) == info.formatFeatureFlags)
			{
				return format;
			}
			else if (info.imageTiling == vk::ImageTiling::eOptimal &&
					 (formatProperties.optimalTilingFeatures & info.formatFeatureFlags) == info.formatFeatureFlags)
			{
				return format;
			}
		}
	}
	return vk::Format::eUndefined;
}
