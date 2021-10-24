#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_queue_family final
{
public:
	vk_queue_family();

	void setup(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface);

	uint32_t getGraphicsIndex() const;

	uint32_t getTransferIndex() const;

	uint32_t getPresentIndex() const;

	vk::SharingMode getSharingMode() const;

	std::vector<uint32_t> getQueueIndexes() const;

	std::vector<uint32_t> getUniqueQueueIndexes() const;

	std::vector<uint32_t> getUniqueQueueIndexes(const vk::SharingMode sharingMode) const;

private:
	uint32_t graphicsIndex;

	uint32_t transferIndex;

	uint32_t presentIndex;

	vk::SharingMode sharingMode;
};