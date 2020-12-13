#pragma once
#include "math/vec3.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"
#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_surface.hxx"

#include <SDL.h>
#include <vector>
#include <vulkan/vulkan.h>

class engine;
class vk_mesh;

class vk_renderer
{
public:
	vk_renderer();
	~vk_renderer();

	static bool isSupported();

	void tick(float deltaTime);

	void createMesh();

	uint32_t getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch = nullptr);

	SDL_Window* getWindow() const;

	VkAllocationCallbacks* getAllocator() const;

	vk_device& getDevice();

	vk_surface& getSurface();

	vk_physical_device& getPhysicalDevice();

	vk_queue_family& getQueueFamily();

protected:
	void drawFrame();

	void createWindow();

	void createInstance();

	void createSwapchain();

	void createImageViews();

	void createRenderPass();

	void createFramebuffers();

	void createCommandPool();

	void createPrimaryCommandBuffers();

	VkCommandBuffer createSecondaryCommandBuffer();

	void createFences();

	void createSemaphores();

	void cleanupSwapchain(VkSwapchainKHR& swapchain);

	void recreateSwapchain();

	void prepareCommandBuffer(uint32_t imageIndex);

	// TODO: should be probably moved or deleted entirely
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

private:
	uint32_t _apiVersion;

	std::vector<vk_mesh*> _meshes;

	VkAllocationCallbacks* _vkAllocator;

	SDL_Window* _window;

	vk_surface _surface;

	vk_physical_device _physicalDevice;

	vk_queue_family _queueFamily;

	vk_device _device;

	VkInstance _vkInstance;

	VkSwapchainKHR _vkSwapchain;

	std::vector<VkImageView> _vkSwapchainImageViews;

	std::vector<VkFramebuffer> _vkFramebuffers;

	VkRenderPass _vkRenderPass;

	VkCommandPool _vkGraphicsCommandPool;
	VkCommandPool _vkTransferCommandPool;

	std::vector<VkCommandBuffer> _vkGraphicsPrimaryCommandBuffers;
	std::vector<VkCommandBuffer> _vkGraphicsSecondaryCommandBuffers;

	std::vector<VkFence> _vkSubmitQueueFences;

	VkSemaphore _vkSepaphoreImageAvaible;

	VkSemaphore _vkSepaphoreRenderFinished;
};
