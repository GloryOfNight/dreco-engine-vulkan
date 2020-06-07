#pragma once
#include "vk_queue_family.hxx"
#include "math/vec3.hxx"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
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

	inline void createSurface();

	inline void selectPhysicalDevice();

	inline void createLogicalDevice();

	inline void setupSurfaceCapabilities();

	inline void createSwapchain();

	inline void createImageViews();

	inline void createRenderPass();

	inline void createFramebuffers();

	inline void createCommandPool();

	inline void createCommandBuffers();

	inline void createPipelineLayout();

	inline void createGraphicsPipeline();

	inline void createShaderModule(const char* src, const size_t& src_size, VkShaderModule& shaderModule);

	inline void recordCommandBuffers();

	inline void createSemaphores();

	inline void cleanupSwapchain(VkSwapchainKHR& swapchain);

	inline void recreateSwapchain();

	void createVertexBuffer();
	void destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createBuffer(VkDeviceSize size, VkBuffer& buffer, VkBufferUsageFlags bufferUsageFlags,
		VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags memoryPropertyFlags);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlags);

private:
	// clang-format off
	std::vector<vec3> vertexes
	{
		vec3(-0.5, 0.5, 0.0),
		vec3(-0.5, -0.5, 0.0),
		vec3(0.5, -0.5, 0.0),

		vec3(-0.5, 0.5, 0.0),
		vec3(0.5, -0.5, 0.0),
		vec3(0.5, 0.5, 0.0) 
	};
	// clang-format on

	engine* _engine;

	SDL_Window* window;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemoryBuffer;

	vk_queue_family queueFamily;

	VkAllocationCallbacks* mAllocator;

	VkInstance mInstance;

	VkSurfaceKHR mSurface;
	VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
	VkSurfaceFormatKHR mSurfaceFormat;

	VkPhysicalDevice mGpu = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties mGpuProperties;
	VkPhysicalDeviceFeatures mGpuFeatures;

	VkDevice mDevice = VK_NULL_HANDLE;

	VkQueue mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue mPresentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
	std::vector<VkImageView> mSwapchainImageViews;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	VkRenderPass mRenderPass;

	VkCommandPool mCommandPool;

	std::vector<VkCommandBuffer> mCommandBuffers;

	VkPipelineLayout mPipelineLayout;

	VkPipeline mPipeline;

	VkSemaphore mSepaphore_Image_Avaible;

	VkSemaphore mSepaphore_Render_Finished;
};
