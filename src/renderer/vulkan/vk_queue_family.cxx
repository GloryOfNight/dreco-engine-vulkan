#include "vk_queue_family.hxx"
#include <vector>

vk_queue_family::vk_queue_family()
	: isSupported{false}
	, graphicsIndex{static_cast<uint32_t>(-1)}
	, transferIndex{static_cast<uint32_t>(-1)}
	, presentIndex{static_cast<uint32_t>(-1)}
	, sharingMode{VK_SHARING_MODE_CONCURRENT}
{
}

vk_queue_family::vk_queue_family(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface)
{
	setup(vkPhysicalDevice, vkSurface);
}

void vk_queue_family::setup(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface)
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
			graphicsIndex = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			if (false == isIndexValid(transferIndex) || graphicsIndex == transferIndex)
			{
				transferIndex = i;
			}
		}

		VkBool32 isQueueFamilySupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &isQueueFamilySupported);
		if (isQueueFamilySupported)
		{
			if (false == isIndexValid(presentIndex) || graphicsIndex == presentIndex || transferIndex == presentIndex)
			{
				presentIndex = i;
			}
		}
	}

	if (isIndexValid(graphicsIndex) && isIndexValid(transferIndex) && isIndexValid(presentIndex))
	{
		isSupported = true;
	}

	if (graphicsIndex == transferIndex && transferIndex == presentIndex)
	{
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
}

bool vk_queue_family::getIsSupported() const
{
	return isSupported;
}

uint32_t vk_queue_family::getGraphicsIndex() const
{
	return graphicsIndex;
}

uint32_t vk_queue_family::getTransferIndex() const
{
	return transferIndex;
}

uint32_t vk_queue_family::getPresentIndex() const
{
	return presentIndex;
}

VkSharingMode vk_queue_family::getSharingMode() const
{
	return sharingMode;
}

bool vk_queue_family::isIndexValid(uint32_t& index)
{
	return static_cast<uint32_t>(-1) != index;
}
