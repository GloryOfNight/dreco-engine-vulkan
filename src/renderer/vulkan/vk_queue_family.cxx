#include "vk_queue_family.hxx"

#include <set>

vk_queue_family::vk_queue_family()
	: isSupported{false}
	, graphicsIndex{UINT32_MAX}
	, transferIndex{UINT32_MAX}
	, presentIndex{UINT32_MAX}
	, sharingMode{VK_SHARING_MODE_CONCURRENT}
	, uniqueQueueIndexes{}
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

	const std::set<uint32_t> indexesSet{presentIndex, graphicsIndex, transferIndex};
	uniqueQueueIndexes = std::vector<uint32_t>(indexesSet.begin(), indexesSet.end());
}

bool vk_queue_family::isVulkanSupported() const
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

const std::vector<uint32_t>& vk_queue_family::getUniqueQueueIndexes() const
{
	return uniqueQueueIndexes;
}

bool vk_queue_family::isIndexValid(uint32_t& index)
{
	return static_cast<uint32_t>(-1) != index;
}
