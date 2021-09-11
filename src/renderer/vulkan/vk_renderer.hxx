#pragma once
#include "core/containers/scene.hxx"
#include "math/vec3.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"
#include "vk_depth_image.hxx"
#include "vk_device.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_msaa_image.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_surface.hxx"
#include "vk_texture_image.hxx"
#include "vk_scene.hxx"

#include <SDL.h>
#include <vector>
#include <vulkan/vulkan.h>

class engine;
class vk_mesh;

class vk_renderer final
{
public:
	vk_renderer();
	vk_renderer(const vk_renderer&) = delete;
	vk_renderer(vk_renderer&&) = delete;
	~vk_renderer();

	vk_renderer& operator=(vk_renderer&) = delete;
	vk_renderer& operator=(vk_renderer&&) = delete;

	static vk_renderer* get();

	static bool isSupported();

	void init();

	void tick(double deltaTime);

	void loadScene(const scene& scn);

	uint32_t getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch = nullptr);

	uint32_t getImageCount() const;

	VkRenderPass getRenderPass() const;

	VkCommandPool getTransferCommandPool() const;

	SDL_Window* getWindow() const;

	VkAllocationCallbacks* getAllocator() const;

	vk_device& getDevice();

	vk_surface& getSurface();

	vk_physical_device& getPhysicalDevice();

	vk_queue_family& getQueueFamily();

	const vk_texture_image& getTextureImagePlaceholder() const;

	VkCommandBuffer beginSingleTimeTransferCommands();

	void submitSingleTimeTransferCommands(VkCommandBuffer commandBuffer);

	void submitSingleTimeTransferCommands(const std::vector<VkSubmitInfo>& submits);

protected:
	void drawFrame();

	void createWindow();

	void createInstance();

	void createSwapchain();

	void createImageViews();

	void createRenderPass();

	void createFramebuffers();

	void createCommandPools();

	void createPrimaryCommandBuffers();

	void createFences();

	void createSemaphores();

	void cleanupSwapchain(VkSwapchainKHR& swapchain);

	void recreateSwapchain();

	VkCommandBuffer prepareCommandBuffer(uint32_t imageIndex);

private:
	uint32_t _apiVersion;

	vk_texture_image _placeholderTextureImage;

	std::vector<vk_scene*> _scenes;

	SDL_Window* _window;

	vk_surface _surface;

	vk_physical_device _physicalDevice;

	vk_queue_family _queueFamily;

	vk_device _device;

	vk_msaa_image _msaaImage;

	vk_depth_image _depthImage;

	VkInstance _vkInstance;

	VkSwapchainKHR _vkSwapchain;

	std::vector<VkImageView> _vkSwapchainImageViews;

	std::vector<VkFramebuffer> _vkFramebuffers;

	VkRenderPass _vkRenderPass;

	std::vector<VkCommandPool> _vkGraphicsCommandPools;
	std::vector<VkCommandBuffer> _vkGraphicsCommandBuffers;

	VkCommandPool _vkTransferCommandPool;

	std::vector<VkFence> _vkSubmitQueueFences;

	VkSemaphore _vkSepaphoreImageAvaible;

	VkSemaphore _vkSepaphoreRenderFinished;
};
