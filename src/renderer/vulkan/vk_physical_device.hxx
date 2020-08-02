#pragma once
#include <vulkan/vulkan.h>

class vk_physical_device
{
public:
	vk_physical_device(const VkInstance* vkInstance);

	void setup(VkSurfaceKHR vkSurface);

	VkPhysicalDevice get() const;

	const VkPhysicalDeviceProperties& getProperties() const;

	const VkPhysicalDeviceFeatures& getFeatures() const;

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const;

private:
	const VkInstance* _vkInstance;

	VkPhysicalDevice _vkPhysicalDevice;
	
	VkPhysicalDeviceProperties _vkPhysicalDeviceProperties;
	
	VkPhysicalDeviceFeatures _vkPhysicalDeviceFeatures;

	VkPhysicalDeviceMemoryProperties _vkPhysicalDeviceMemoryProperties;
};