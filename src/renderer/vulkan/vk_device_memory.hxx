#pragma once
#include <vulkan/vulkan.h>

enum class vk_device_memory_properties : VkFlags
{
	HOST = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	DEVICE_ONLY = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
};

class vk_device_memory final
{
public:
	vk_device_memory();
	vk_device_memory(const vk_device_memory&) = delete;
	vk_device_memory(vk_device_memory&&) = delete;
	~vk_device_memory();

	vk_device_memory& operator=(const vk_device_memory&) = delete;
	vk_device_memory& operator=(vk_device_memory&&) = delete;

	void allocate(const VkMemoryRequirements& vkMemoryRequirements, const VkMemoryPropertyFlags vkMemoryPropertyFlags);

	void free();

	void map(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0);

	VkDeviceMemory get() const;

protected:
	static uint32_t findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& vkMemoryProperties,
		uint32_t memoryTypeBits, VkMemoryPropertyFlags vkMemoryPropertyFlags);

private:
	VkDeviceMemory _vkDeviceMemory;
};