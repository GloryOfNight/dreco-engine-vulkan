#pragma once
#include <vulkan/vulkan_core.h>

class vk_queue_family
{
public:
	vk_queue_family()
		: isSupported{false}
		, graphicsQueueFamilyIndex{static_cast<uint32_t>(-1)}
		, presentQueueFamilyIndex{static_cast<uint32_t>(-1)}
	{
	}

	vk_queue_family(const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface) : vk_queue_family()
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
		VkQueueFamilyProperties queueFamilyProperties[queueFamilyCount];
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProperties);

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& queueFamily = queueFamilyProperties[i];

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueueFamilyIndex = i;
			}
			
			VkBool32 isQueueFamilySupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &isQueueFamilySupported);
			if (isQueueFamilySupported)
			{
				presentQueueFamilyIndex = i;
			}

			if (graphicsQueueFamilyIndex != static_cast<uint32_t>(-1) &&
				presentQueueFamilyIndex != static_cast<uint32_t>(-1))
			{
				isSupported = true;
				break;
			}
		}
	}

	bool isSupported;

	uint32_t graphicsQueueFamilyIndex;

	uint32_t presentQueueFamilyIndex;
};