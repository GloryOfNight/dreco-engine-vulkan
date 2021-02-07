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

void vk_buffer::copyBufferToImage(VkBuffer vkBuffer, VkImage vkImage, VkImageLayout vkImageLayout, uint32_t width, uint32_t height)
{
	vk_renderer* renderer{vk_renderer::get()};
	vk_device& device{renderer->getDevice()};
	VkCommandPool transferCommandPool{renderer->getTransferCommandPool()};

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = transferCommandPool;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(renderer->getDevice().get(), &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
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

	vkCmdCopyBufferToImage(commandBuffer, vkBuffer, vkImage, vkImageLayout, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getTransferQueue());
	vkFreeCommandBuffers(device.get(), transferCommandPool, 1, &commandBuffer);
}

void vk_buffer::createBuffer(const VkDevice vkDevice, const vk_buffer_create_info& create_info)
{
	vk_queue_family& queueFamily{vk_renderer::get()->getQueueFamily()};

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = create_info.size;
	bufferCreateInfo.usage = static_cast<VkBufferUsageFlagBits>(create_info.usage);
	bufferCreateInfo.sharingMode = queueFamily.getSharingMode();

	const auto queueFamilyIndexes = queueFamily.getUniqueQueueIndexes();
	bufferCreateInfo.queueFamilyIndexCount = queueFamilyIndexes.size();
	bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndexes.data();

	VK_CHECK(vkCreateBuffer(vkDevice, &bufferCreateInfo, vkGetAllocator(), &_vkBuffer));
}

void vk_buffer::bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset)
{
	vkBindBufferMemory(vkDevice, _vkBuffer, vkDeviceMemory, memoryOffset);
}
