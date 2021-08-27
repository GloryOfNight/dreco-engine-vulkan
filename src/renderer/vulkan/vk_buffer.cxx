#include "vk_buffer.hxx"

#include "vk_allocator.hxx"
#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

#include <cstring>

vk_buffer::vk_buffer()
	: _vkBuffer{VK_NULL_HANDLE}
{
}

vk_buffer::~vk_buffer()
{
	destroy();
}

void vk_buffer::create(const vk_buffer_create_info& create_info)
{
	const VkDevice vkDevice{vk_renderer::get()->getDevice().get()};
	createBuffer(vkDevice, create_info);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vkDevice, _vkBuffer, &memoryRequirements);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(create_info.memory_properties_flags));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);
}

void vk_buffer::destroy()
{
	if (VK_NULL_HANDLE != _vkBuffer)
	{
		vkDestroyBuffer(vk_renderer::get()->getDevice().get(), _vkBuffer, vkGetAllocator());
		_vkBuffer = VK_NULL_HANDLE;
	}
	_deviceMemory.free();
}

VkBuffer vk_buffer::get() const
{
	return _vkBuffer;
}

vk_device_memory& vk_buffer::getDeviceMemory()
{
	return _deviceMemory;
}

void vk_buffer::copyBuffer(const VkBuffer vkBufferSrc, const VkBuffer VkBufferDst, const std::vector<VkBufferCopy>& vkBufferCopyRegions)
{
	vk_renderer* renderer{vk_renderer::get()};

	VkCommandBuffer vkCommandBuffer{renderer->beginSingleTimeTransferCommands()};

	vkCmdCopyBuffer(vkCommandBuffer, vkBufferSrc, VkBufferDst, vkBufferCopyRegions.size(), vkBufferCopyRegions.data());

	renderer->endSingleTimeTransferCommands(vkCommandBuffer);
}

void vk_buffer::copyBufferToImage(const VkBuffer vkBuffer, const VkImage vkImage, const VkImageLayout vkImageLayout, const uint32_t width, const uint32_t height)
{
	vk_renderer* renderer{vk_renderer::get()};

	VkCommandBuffer vkCommandBuffer{renderer->beginSingleTimeTransferCommands()};

	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageOffset = {0, 0, 0};
	copyRegion.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(vkCommandBuffer, vkBuffer, vkImage, vkImageLayout, 1, &copyRegion);

	renderer->endSingleTimeTransferCommands(vkCommandBuffer);
}

void vk_buffer::createBuffer(const VkDevice vkDevice, const vk_buffer_create_info& create_info)
{
	const vk_queue_family& queueFamily{vk_renderer::get()->getQueueFamily()};
	std::vector<uint32_t> queueIndexes;

	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.size = create_info.size;
	createInfo.usage = static_cast<VkBufferUsageFlagBits>(create_info.usage);
	createInfo.sharingMode = queueFamily.getSharingMode();
	if (VK_SHARING_MODE_CONCURRENT == createInfo.sharingMode)
	{
		queueIndexes = queueFamily.getUniqueQueueIndexes();
		createInfo.queueFamilyIndexCount = queueIndexes.size();
		createInfo.pQueueFamilyIndices = queueIndexes.data();
	}

	VK_CHECK(vkCreateBuffer(vkDevice, &createInfo, vkGetAllocator(), &_vkBuffer));
}

void vk_buffer::bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset)
{
	vkBindBufferMemory(vkDevice, _vkBuffer, vkDeviceMemory, memoryOffset);
}
