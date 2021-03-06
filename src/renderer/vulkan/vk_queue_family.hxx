#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class vk_queue_family
{
public:
	vk_queue_family();

	vk_queue_family(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface);

	void setup(const VkPhysicalDevice& vkPhysicalDevice, const VkSurfaceKHR& vkSurface);

	bool isVulkanSupported() const;

	uint32_t getGraphicsIndex() const;

	uint32_t getTransferIndex() const;

	uint32_t getPresentIndex() const;

	VkSharingMode getSharingMode() const;

	std::vector<uint32_t> getQueueIndexes() const;

	std::vector<uint32_t> getUniqueQueueIndexes() const;

private:
	bool isSupported;

	uint32_t graphicsIndex;

	uint32_t transferIndex;

	uint32_t presentIndex;

	VkSharingMode sharingMode;
};