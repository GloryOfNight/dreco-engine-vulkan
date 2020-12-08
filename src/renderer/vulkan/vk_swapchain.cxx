#include "vk_swapchain.hxx"

#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_surface.hxx"
#include "vk_utils.hxx"

vk_swapchain::vk_swapchain(vk_renderer& renderer)
	: _renderer{renderer}
	, _vkSwapchain{VK_NULL_HANDLE}
{
}

void vk_swapchain::create()
{
	const vk_queue_family& queueFamily = _renderer.getQueueFamily();
	const vk_surface& surface = _renderer.getSurface();
	VkDevice device = _renderer.getDevice().get();
	VkAllocationCallbacks* allocator = _renderer.getAllocator();

	createSwapchain(queueFamily, surface, device, allocator);
	createSwapchainImageViews(device, surface.getFormat().format, allocator);
}

uint32_t vk_swapchain::getImageCount() const
{
	return _vkSwapchainImageViews.size();
}

void vk_swapchain::createSwapchain(const vk_queue_family& queueFamily, const vk_surface& surface, VkDevice device, VkAllocationCallbacks* allocator)
{
	VkSharingMode sharingMode{queueFamily.getSharingMode()};
	std::vector<uint32_t> queueFamilyIndexes{
		queueFamily.getGraphicsIndex(),
		queueFamily.getTransferIndex(),
		queueFamily.getPresentIndex()};

	const VkSurfaceFormatKHR& surfaceFormat{surface.getFormat()};
	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{surface.getCapabilities()};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface.get();
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = sharingMode;
	swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndexes.size());
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes.data();
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = surface.getPresentMode();
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = _vkSwapchain;

	const VkResult createResult =
		vkCreateSwapchainKHR(device, &swapchainCreateInfo, allocator, &_vkSwapchain);

	if (VK_SUCCESS == createResult && swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		//cleanupSwapchain(swapchainCreateInfo.oldSwapchain); TODO
	}
}

void vk_swapchain::createSwapchainImageViews(VkDevice device, VkFormat surfaceFormat, VkAllocationCallbacks* allocator)
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, _vkSwapchain, &imageCount, nullptr);

	std::vector<VkImage> vkSwapchainImages(imageCount);
	vkGetSwapchainImagesKHR(device, _vkSwapchain, &imageCount, vkSwapchainImages.data());

	_vkSwapchainImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = vkSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = surfaceFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, allocator, &_vkSwapchainImageViews[i]));
	}
}
