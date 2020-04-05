#pragma once
#include <vulkan/vulkan_core.h>

class vk_queue_family
{
public:
	vk_queue_family()
	{
	}
	uint32_t mIdxGraphicsFamily = -1;
	uint32_t mIdxPresentFamily = -1;

	void findQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
		VkQueueFamilyProperties queueFamilyProperties[queueFamilyCount];
		vkGetPhysicalDeviceQueueFamilyProperties(
			gpu, &queueFamilyCount, queueFamilyProperties);

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				mIdxGraphicsFamily = i;
			}

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supported);
			if (queueFamilyProperties[i].queueCount > 0 && supported)
			{
				mIdxPresentFamily = i;
			}

			if (mIdxGraphicsFamily != static_cast<uint32_t>(-1) &&
				mIdxPresentFamily != static_cast<uint32_t>(-1))
			{
				break;
			}
		}
	}
};