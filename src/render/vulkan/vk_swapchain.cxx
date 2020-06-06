#include "vk_swapchain.hxx"
#include "vk_check.hxx"

vk_swapchain::vk_swapchain(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	setupSurfaceFormats(gpu, surface);
	setupPresentModes(gpu, surface);
}

VkSurfaceFormatKHR vk_swapchain::getSurfaceFormat() const
{
	return _surfaceFormat;
}

VkPresentModeKHR vk_swapchain::getPresentMode() const
{
	return _presentMode;
}

void vk_swapchain::setupSurfaceFormats(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
	VkSurfaceFormatKHR mSurfaceFormats[formatCount];
	vk_checkError(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, mSurfaceFormats));

	for (auto& surfaceFormat : mSurfaceFormats) 
	{
		_surfaceFormat = surfaceFormat;
		if(surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) 
		{
			break;
		}
	}
}

void vk_swapchain::setupPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, nullptr);
	VkPresentModeKHR mPresentModes[presentCount];
	vk_checkError(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, mPresentModes));

	for (auto& presentMode : mPresentModes) 
	{
		_presentMode = presentMode;
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
		{
			break;
		}
	}
}