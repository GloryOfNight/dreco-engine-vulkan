#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

class vk_swapchain
{
public:
	vk_swapchain(VkPhysicalDevice gpu, VkSurfaceKHR surface);

	VkSurfaceCapabilitiesKHR mCapabilities;
	std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
	std::vector<VkPresentModeKHR> mPresentModes;

	VkSurfaceFormatKHR getSurfaceFormat() const;

	VkPresentModeKHR getPresentMode() const;
private:
	inline void setupCapabilities(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	inline void setupSurfaceFormats(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	inline void setupPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface);
};