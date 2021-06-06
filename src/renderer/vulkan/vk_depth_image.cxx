#include "vk_depth_image.hxx"

#include "vk_renderer.hxx"

void vk_depth_image::create()
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	_format = findDepthFormat();
	const VkExtent2D vkExtent{renderer->getSurface().getCapabilities().currentExtent};

	createImage(vkDevice, _format, vkExtent.width, vkExtent.height);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);

	createImageView(vkDevice, _format);
	createSampler(vkDevice);

	transitionImageLayout(_vkImage, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, getImageAspectFlags());
}

void vk_depth_image::recreate()
{
	destroy();
	create();
}

VkFormat vk_depth_image::getFormat() const
{
	return _format;
}

VkImageAspectFlags vk_depth_image::getImageAspectFlags() const
{
	return hasStencilComponent() ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
}

VkImageUsageFlags vk_depth_image::getImageUsageFlags() const
{
	return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

VkFormat vk_depth_image::findDepthFormat() const
{
	return vk_renderer::get()->getPhysicalDevice().findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool vk_depth_image::hasStencilComponent() const
{
	return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
}