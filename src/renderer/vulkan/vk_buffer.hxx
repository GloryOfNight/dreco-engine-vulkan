#pragma once
#include <stdint.h>
#include <vulkan/vulkan.h>

class vk_device;
class vk_physical_device;
class vk_queue_family;

enum class vk_buffer_usage : VkFlags
{
	UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
};

enum class vk_buffer_memory_properties : VkFlags
{
	HOST = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	DEVICE = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
};

struct vk_buffer_create_info
{
	vk_buffer_usage usage;
	vk_buffer_memory_properties memory_properties;
	const vk_queue_family* queueFamily;
	const vk_physical_device* physicalDevice;
	VkDeviceSize size;
};

class vk_buffer
{
public:
	vk_buffer();
	~vk_buffer();

	void create(const vk_device* device, const vk_buffer_create_info& create_info);

	void destroy();

	void map(const void* data, const VkDeviceSize size);

	VkBuffer get() const;

protected:
	static void destroy(VkDevice vkDevice, VkBuffer& vkBuffer, VkDeviceMemory& vkDeviceMemery);

	static void createBuffer(const vk_buffer_create_info& create_info, VkBuffer& vkBuffer, VkDevice vkDevice, VkDeviceMemory& vkDeviceMemory);

	static uint32_t findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& vkPhysicalDeviceMemoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags vkMemoryPropertyFlags);

private:
	const vk_device* _device;

	VkBuffer _vkBuffer;

	VkDeviceMemory _vkDeviceMemory;
};