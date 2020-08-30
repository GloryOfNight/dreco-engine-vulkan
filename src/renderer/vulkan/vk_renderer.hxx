#pragma once
#include "vk_queue_family.hxx"
#include "vk_surface.hxx"
#include "vk_physical_device.hxx"
#include "vk_device.hxx"
#include "vk_buffer.hxx"
#include "vk_mesh.hxx"

#include "math/vec3.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <vector>

class engine;

class vk_renderer
{
public:
	vk_renderer(engine* eng);
	~vk_renderer();

	void tick(const float& delta_time);

	SDL_Window* getWindow() const;

protected:
	void drawFrame();

	inline void createWindow();

	inline void createInstance();

	inline void createSwapchain();

	inline void createImageViews();

	inline void createRenderPass();

	inline void createFramebuffers();

	inline void createCommandPool();

	inline void createCommandBuffers();

	inline void recordCommandBuffers();

	inline void createSemaphores();

	inline void cleanupSwapchain(VkSwapchainKHR& swapchain);

	inline void recreateSwapchain();

	// TODO: should be probably moved or deleted entirely
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

private:

	vk_mesh vkMesh;

	engine* _engine;

	VkAllocationCallbacks* mAllocator;
	
	SDL_Window* window;

	vk_surface surface;
	
	vk_physical_device physicalDevice;

	vk_queue_family queueFamily;

	vk_device device;

	VkInstance mInstance;

	VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;

	std::vector<VkImageView> mSwapchainImageViews;

	std::vector<VkFramebuffer> mFramebuffers;

	VkRenderPass mRenderPass;

	VkCommandPool mGraphicsCommandPool;
	VkCommandPool mTransferCommandPool;

	std::vector<VkCommandBuffer> mGraphicsCommandBuffers;

	VkSemaphore mSepaphore_Image_Avaible;

	VkSemaphore mSepaphore_Render_Finished;
};
