#include "vk_utils.hxx"

vk::Format vk_utils::findSupportedFormat(const vk::PhysicalDevice physicalDevice, const find_supported_format_info& info) noexcept
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
