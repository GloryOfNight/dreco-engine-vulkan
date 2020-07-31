#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <set>

class vk_swapchain
{
public:
	vk_swapchain(VkPhysicalDevice gpu, VkSurfaceKHR surface);

	VkSurfaceFormatKHR getSurfaceFormat() const;

	VkPresentModeKHR getPresentMode() const;

private:

	inline void setupSurfaceFormats(VkPhysicalDevice gpu, VkSurfaceKHR surface);

	inline void setupPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface);

	VkSurfaceFormatKHR _surfaceFormat;

	VkPresentModeKHR _presentMode;

};