#pragma once
#include "vk_device_memory.hxx"

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

class vk_device;
class vk_physical_device;
class vk_queue_family;

struct vk_buffer_create_info
{
	VkFlags usage;
	vk_device_memory_properties memory_properties_flags;
	VkDeviceSize size;
};

class vk_buffer final
{
public:
	vk_buffer();
	vk_buffer(const vk_buffer&) = delete;
	vk_buffer(vk_buffer&&) = delete;
	~vk_buffer();

	vk_buffer& operator=(const vk_buffer&) = delete;
	vk_buffer& operator=(vk_buffer&&) = delete;

	void create(const vk_buffer_create_info& create_info);

	virtual void destroy();

	VkBuffer get() const;

	vk_device_memory& getDeviceMemory();

	static void copyBuffer(const VkBuffer vkBufferSrc, const VkBuffer VkBufferDst, const std::vector<VkBufferCopy>& vkBufferCopyRegions);

	static void copyBufferToImage(const VkBuffer vkBuffer, const VkImage vkImage, const VkImageLayout vkImageLayout, const uint32_t width, const uint32_t height);

protected:
	void createBuffer(const VkDevice vkDevice, const vk_buffer_create_info& create_info);

	void bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset);

private:
	vk_device_memory _deviceMemory;

	VkBuffer _vkBuffer;
};