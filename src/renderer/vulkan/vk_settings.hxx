#pragma once
#include <vulkan/vulkan.h>

class vk_renderer;

struct vk_settings
{
	vk_settings();
	void init(const vk_renderer* renderer);

	VkSampleCountFlagBits getMaxSampleCount() const;

	VkSampleCountFlagBits getPrefferedSampleCount() const;
	bool setPrefferedSampleCount(const VkSampleCountFlagBits sampleCount);

	bool getIsSamplingSupported() const;

	VkPolygonMode getDefaultPolygonMode() const;
	bool setDefaultPolygonMode(const VkPolygonMode mode);

private:
	VkSampleCountFlagBits _maxSampleCount;
	VkSampleCountFlagBits _prefferedSampleCount;
	
	VkPolygonMode _polygonMode;
};