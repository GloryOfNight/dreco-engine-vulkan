#include "vk_queue_family.hxx"

#include <set>

#define IS_QUEUE_INDEX_VALID(index) (UINT32_MAX != index)

vk_queue_family::vk_queue_family()
	: isSupported{false}
	, graphicsIndex{UINT32_MAX}
	, transferIndex{UINT32_MAX}
	, presentIndex{UINT32_MAX}
	, sharingMode{VK_SHARING_MODE_EXCLUSIVE}
{
}

vk_queue_family::vk_queue_family(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface)
	: vk_queue_family()
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

		const auto queueFlags = queueFamily.queueFlags;
		if ((queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			VkBool32 isQueueFamilySupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &isQueueFamilySupported);
			if (isQueueFamilySupported)
			{
				graphicsIndex = transferIndex = presentIndex = i;
				break;
			}
		}
	}

	if (IS_QUEUE_INDEX_VALID(graphicsIndex) && IS_QUEUE_INDEX_VALID(transferIndex) && IS_QUEUE_INDEX_VALID(presentIndex))
	{
		isSupported = true;
	}
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

std::vector<uint32_t> vk_queue_family::getQueueIndexes() const
{
	return std::vector<uint32_t>{presentIndex, graphicsIndex, transferIndex};
}

std::vector<uint32_t> vk_queue_family::getUniqueQueueIndexes() const
{
	const std::set<uint32_t> indexesSet{presentIndex, graphicsIndex, transferIndex};
	return std::vector<uint32_t>(indexesSet.begin(), indexesSet.end());
}

std::vector<uint32_t> vk_queue_family::getUniqueQueueIndexes(const VkSharingMode sharingMode) const
{
	return sharingMode == VK_SHARING_MODE_CONCURRENT ? getUniqueQueueIndexes() : std::vector<uint32_t>{presentIndex};
}