#pragma once
#include "core/containers/scene.hxx"
#include "math/vec3.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"
#include "vk_depth_image.hxx"
#include "vk_device.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_msaa_image.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_scene.hxx"
#include "vk_settings.hxx"
#include "vk_surface.hxx"
#include "vk_texture_image.hxx"

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

	const vk_device& getDevice() const { return _device; }
	vk_device& getDevice() { return _device; }

	const vk_surface& getSurface() const { return _surface; }
	vk_surface& getSurface() { return _surface; }

	const vk_physical_device& getPhysicalDevice() const { return _physicalDevice; }
	vk_physical_device& getPhysicalDevice() { return _physicalDevice; }

	const vk_queue_family& getQueueFamily() const { return _queueFamily; }
	vk_queue_family& getQueueFamily() { return _queueFamily; }

	const vk_settings& getSettings() const { return _settings; }
	vk_settings& getSettings() { return _settings; }

	const vk_texture_image& getTextureImagePlaceholder() const { return _placeholderTextureImage; }

	VkCommandBuffer beginSingleTimeTransferCommands();

	void submitSingleTimeTransferCommands(VkCommandBuffer commandBuffer);

	void submitSingleTimeTransferCommands(const std::vector<VkSubmitInfo>& submits);

	void applySettings();

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

	vk_device _device;

	vk_queue_family _queueFamily;

	vk_settings _settings;

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

	VkSemaphore _vkSemaphoreImageAvaible;

	VkSemaphore _vkSemaphoreRenderFinished;
};
