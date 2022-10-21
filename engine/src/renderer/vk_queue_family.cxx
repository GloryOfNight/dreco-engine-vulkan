#include "vk_queue_family.hxx"

#include <set>

#define IS_QUEUE_INDEX_VALID(index) (UINT32_MAX != index)

void vk_queue_family::setup(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
	for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
	{
		const auto queueFlags = queueFamilyProperties[i].queueFlags;
		if ((queueFlags & vk::QueueFlagBits::eGraphics) && (queueFlags & vk::QueueFlagBits::eTransfer))
		{
			const vk::Bool32 isSupported = physicalDevice.getSurfaceSupportKHR(i, surface);
			if (isSupported)
			{
				graphicsIndex = transferIndex = presentIndex = i;
				break;
			}
		}
	}
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

vk::SharingMode vk_queue_family::getSharingMode() const
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

std::vector<uint32_t> vk_queue_family::getUniqueQueueIndexes(const vk::SharingMode sharingMode) const
{
	return sharingMode == vk::SharingMode::eConcurrent ? getUniqueQueueIndexes() : std::vector<uint32_t>{presentIndex};
}