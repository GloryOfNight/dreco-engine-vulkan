#include "vk_image.hxx"

#include "vk_allocator.hxx"
#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

void vk_image::destroy() 
{
    const VkDevice vkDevice{vk_renderer::get()->getDevice().get()};
	if (VK_NULL_HANDLE != _vkImageView)
	{
		vkDestroyImageView(vkDevice, _vkImageView, vkGetAllocator());
		_vkImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != _vkImage)
	{
		vkDestroyImage(vkDevice, _vkImage, vkGetAllocator());
		_vkImage = VK_NULL_HANDLE;
	}
	_deviceMemory.free();
}

VkImageAspectFlags vk_image::getImageAspectFlags() const
{
	return 0;
}

VkImageUsageFlags vk_image::getImageUsageFlags() const
{
	return 0;
}

void vk_image::createImage(const VkDevice vkDevice, const VkFormat vkFormat, const uint32_t width, const uint32_t height, const VkSampleCountFlagBits samples)
{
	const vk_queue_family& queueFamily{vk_renderer::get()->getQueueFamily()};
	std::vector<uint32_t> queueIndexes;

	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = vkFormat;
	createInfo.extent = VkExtent3D{width, height, 1};
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = samples;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = getImageUsageFlags();
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.sharingMode = queueFamily.getSharingMode();
	if (VK_SHARING_MODE_CONCURRENT == createInfo.sharingMode)
	{
		queueIndexes = queueFamily.getUniqueQueueIndexes();
		createInfo.queueFamilyIndexCount = queueIndexes.size();
		createInfo.pQueueFamilyIndices = queueIndexes.data();
	}

	VK_CHECK(vkCreateImage(vkDevice, &createInfo, vkGetAllocator(), &_vkImage));
}

void vk_image::bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset)
{
	VK_CHECK(vkBindImageMemory(vkDevice, _vkImage, vkDeviceMemory, memoryOffset));
}

void vk_image::createImageView(const VkDevice vkDevice, const VkFormat vkFormat)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.image = _vkImage;
	createInfo.format = vkFormat;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.subresourceRange.aspectMask = getImageAspectFlags();
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;

	VK_CHECK(vkCreateImageView(vkDevice, &createInfo, vkGetAllocator(), &_vkImageView));
}

void vk_image::transitionImageLayout(const VkImage vkImage, const VkFormat vkFormat, const VkImageLayout vkLayoutOld, const VkImageLayout vkLayoutNew,
	const VkAccessFlags vkAccessFlagsSrc, const VkAccessFlags vkAccessFlagsDst,
	const VkPipelineStageFlags vkPipelineStageFlagsSrc, const VkPipelineStageFlags vkPipelineStageFlagsDst, const VkImageAspectFlags vkAspectFlags)
{
	vk_renderer* renderer{vk_renderer::get()};
	const vk_queue_family& queueFamily{renderer->getQueueFamily()};
	VkCommandBuffer vkCommandBuffer = renderer->beginSingleTimeTransferCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = vkLayoutOld;
	barrier.newLayout = vkLayoutNew;
	barrier.srcQueueFamilyIndex = queueFamily.getSharingMode() == VK_SHARING_MODE_CONCURRENT ? VK_QUEUE_FAMILY_IGNORED : queueFamily.getGraphicsIndex();
	barrier.dstQueueFamilyIndex = queueFamily.getSharingMode() == VK_SHARING_MODE_CONCURRENT ? VK_QUEUE_FAMILY_IGNORED : queueFamily.getGraphicsIndex();
	barrier.image = vkImage;
	barrier.subresourceRange.aspectMask = vkAspectFlags;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = vkAccessFlagsSrc;
	barrier.dstAccessMask = vkAccessFlagsDst;

	vkCmdPipelineBarrier(vkCommandBuffer, vkPipelineStageFlagsSrc, vkPipelineStageFlagsDst, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	renderer->endSingleTimeTransferCommands(vkCommandBuffer);
}