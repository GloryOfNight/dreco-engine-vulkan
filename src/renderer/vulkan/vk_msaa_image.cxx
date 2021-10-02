#include "vk_msaa_image.hxx"

#include "vk_renderer.hxx"
#include "vk_surface.hxx"

void vk_msaa_image::create()
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	const VkFormat format{renderer->getSurface().getFormat().format};
	const VkSampleCountFlagBits samples{renderer->getSettings().getPrefferedSampleCount()};
	const VkExtent2D vkExtent{renderer->getSurface().getCapabilities().currentExtent};

	createImage(vkDevice, format, vkExtent.width, vkExtent.height, samples);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);

	createImageView(vkDevice, format);
}

void vk_msaa_image::recreate() 
{
	destroy();
	create();
}

VkImageAspectFlags vk_msaa_image::getImageAspectFlags() const
{
	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageUsageFlags vk_msaa_image::getImageUsageFlags() const
{
	return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}