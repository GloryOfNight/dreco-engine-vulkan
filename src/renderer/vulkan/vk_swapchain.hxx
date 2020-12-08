#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class vk_renderer;
class vk_queue_family;
class vk_surface;

class vk_swapchain
{
public:
	vk_swapchain(vk_renderer& renderer);

	void create();

	uint32_t getImageCount() const;

protected:
	void createSwapchain(const vk_queue_family& queueFamily, const vk_surface& surface, VkDevice device, VkAllocationCallbacks* allocator);

	void createSwapchainImageViews(VkDevice device, VkFormat surfaceFormat, VkAllocationCallbacks* allocator);

private:
	vk_renderer& _renderer;

	VkSwapchainKHR _vkSwapchain;

	std::vector<VkImageView> _vkSwapchainImageViews;
};