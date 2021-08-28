#pragma once
#include "math/vec3.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"
#include "vk_depth_image.hxx"
#include "vk_device.hxx"
#include "vk_msaa_image.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_surface.hxx"

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

	vk_mesh* createMesh(const mesh_data& meshData);

	VkCommandBuffer createSecondaryCommandBuffer();

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

	VkCommandBuffer beginSingleTimeGraphicsCommands();

	void endSingleTimeGraphicsCommands(const VkCommandBuffer vkCommandBuffer);

	VkCommandBuffer beginSingleTimeTransferCommands();

	void endSingleTimeTransferCommands(const VkCommandBuffer vkCommandBuffer);

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

	void createFences();

	void createSemaphores();

	void cleanupSwapchain(VkSwapchainKHR& swapchain);

	void recreateSwapchain();

	void prepareCommandBuffer(uint32_t imageIndex);

private:
	uint32_t _apiVersion;

	std::vector<vk_mesh*> _meshes;

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

	VkCommandPool _vkGraphicsCommandPool;
	VkCommandPool _vkTransferCommandPool;

	std::vector<VkCommandBuffer> _vkGraphicsPrimaryCommandBuffers;
	std::vector<VkCommandBuffer> _vkGraphicsSecondaryCommandBuffers;

	std::vector<VkFence> _vkSubmitQueueFences;

	VkSemaphore _vkSepaphoreImageAvaible;

	VkSemaphore _vkSepaphoreRenderFinished;
};
