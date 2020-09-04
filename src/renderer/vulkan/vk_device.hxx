#pragma once
#include <vulkan/vulkan.h>

class vk_physical_device;
class vk_queue_family;

class vk_device
{
public:
	vk_device();
	~vk_device();

	void create(const vk_physical_device& vkPhysicalDevice, const vk_queue_family& vkQueueFamily);

	void waitIdle();

	void destroy();

	VkDevice get() const;

	VkQueue getGraphicsQueue() const;

	VkQueue getPresentQueue() const;

	VkQueue getTransferQueue() const;

private:
	VkDevice _vkDevice;

	VkQueue _vkGraphicsQueue;

	VkQueue _vkPresentQueue;

	VkQueue _vkTransferQueue;
};