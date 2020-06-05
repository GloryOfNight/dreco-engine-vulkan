#include "vk_swapchain.hxx"
#include "vk_check.hxx"

vk_swapchain::vk_swapchain(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	setupSurfaceFormats(gpu, surface);
	setupPresentModes(gpu, surface);
}

VkSurfaceFormatKHR vk_swapchain::getSurfaceFormat() const
{
	return mSurfaceFormats[0];
}

VkPresentModeKHR vk_swapchain::getPresentMode() const
{
	for (const auto i : mPresentModes)
	{
		if (i == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			return i;
		}
	}

	return mPresentModes[0];
}

void vk_swapchain::setupSurfaceFormats(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
	mSurfaceFormats.resize(formatCount);
	vk_checkError(vkGetPhysicalDeviceSurfaceFormatsKHR(
		gpu, surface, &formatCount, mSurfaceFormats.data()));
}

void vk_swapchain::setupPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		gpu, surface, &presentCount, nullptr);
	mPresentModes.resize(presentCount);
	vk_checkError(vkGetPhysicalDeviceSurfacePresentModesKHR(
		gpu, surface, &presentCount, mPresentModes.data()));
}