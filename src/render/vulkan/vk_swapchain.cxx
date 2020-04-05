#include "vk_swapchain.hxx"
#include "vk_check.hxx"

vk_swapchain::vk_swapchain(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	setupCapabilities(gpu, surface);
	setupSurfaceFormats(gpu, surface);
	setupPresentModes(gpu, surface);
}

VkSurfaceFormatKHR vk_swapchain::getSurfaceFormat() const
{
	for (const auto i : mSurfaceFormats)
	{
		if (i.format == VK_FORMAT_B8G8R8_UNORM &&
			i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return i;
		}
	}

	return mSurfaceFormats[0];
}

VkPresentModeKHR vk_swapchain::getPresentMode() const
{
	for (const auto i : mPresentModes)
	{
		if (i == VK_PRESENT_MODE_FIFO_KHR)
		{
			return i;
		}
	}

	return mPresentModes[0];
}

void vk_swapchain::setupCapabilities(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	vk_checkError(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &mCapabilities));
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