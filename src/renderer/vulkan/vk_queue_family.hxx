#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>

class vk_queue_family
{
public:
	vk_queue_family()
		: isSupported{false}
		, graphicsQueueFamilyIndex{static_cast<uint32_t>(-1)}
		, transferQueueFamilyIndex{static_cast<uint32_t>(-1)}
		, presentQueueFamilyIndex{static_cast<uint32_t>(-1)}
		, sharingMode{VK_SHARING_MODE_CONCURRENT}
	{
	}

	vk_queue_family(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface) : vk_queue_family()
	{
		setup(vkPhysicalDevice, vkSurface);
	}

	void setup(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface)
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& queueFamily = queueFamilyProperties[i];

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueueFamilyIndex = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				if (false == isIndexValid(transferQueueFamilyIndex) ||
					graphicsQueueFamilyIndex == transferQueueFamilyIndex)
				{
					transferQueueFamilyIndex = i;
				}
			}

			VkBool32 isQueueFamilySupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &isQueueFamilySupported);
			if (isQueueFamilySupported)
			{
				if (false == isIndexValid(presentQueueFamilyIndex) ||
					graphicsQueueFamilyIndex == presentQueueFamilyIndex ||
					transferQueueFamilyIndex == presentQueueFamilyIndex)
				{
					presentQueueFamilyIndex = i;
				}
			}
		}

		if (isIndexValid(graphicsQueueFamilyIndex) && isIndexValid(transferQueueFamilyIndex) &&
			isIndexValid(presentQueueFamilyIndex))
		{
			isSupported = true;
		}

		if (graphicsQueueFamilyIndex == transferQueueFamilyIndex && transferQueueFamilyIndex == presentQueueFamilyIndex)
		{
			sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
	}

	bool isIndexValid(uint32_t& index)
	{
		return static_cast<uint32_t>(-1) != index;
	}

	bool isSupported;

	uint32_t graphicsQueueFamilyIndex;

	uint32_t transferQueueFamilyIndex;

	uint32_t presentQueueFamilyIndex;

	VkSharingMode sharingMode;
};