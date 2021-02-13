#pragma once
#include <vulkan/vulkan.h>

class vk_physical_device final
{
public:
	vk_physical_device();

	void setup(const VkInstance vkInstance, VkSurfaceKHR vkSurface);

	VkPhysicalDevice get() const;

	const VkPhysicalDeviceProperties& getProperties() const;

	const VkPhysicalDeviceFeatures& getFeatures() const;

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const;

private:
	VkPhysicalDevice _vkPhysicalDevice;

	VkPhysicalDeviceProperties _vkPhysicalDeviceProperties;

	VkPhysicalDeviceFeatures _vkPhysicalDeviceFeatures;

	VkPhysicalDeviceMemoryProperties _vkPhysicalDeviceMemoryProperties;
};