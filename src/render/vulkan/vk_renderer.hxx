#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

class engine;

class vk_renderer
{
public:
	vk_renderer(engine* eng);
	~vk_renderer();

	void tick(const float& delta_time);

	GLFWwindow* getWindow() const;

	static bool isSupported();

protected:
	inline void createWindow();

	inline void createInstance();

	inline void createSurface();

	inline void selectPhysicalDevice();

	inline void createLogicalDevice();

	inline void createSwapchain();

	inline void createImageViews();

	inline void createRenderPass();

	inline void createFramebuffers();

	inline void createCommandPool();

	inline void createCommandBuffers();

	inline void createGraphicsPipeline();

	inline void createShaderModule(const char* src, const size_t& src_size, VkShaderModule& shaderModule);
private:
	engine* _engine;

	GLFWwindow* window;

	VkAllocationCallbacks* mAllocator;

	VkInstance mInstance;

	VkSurfaceKHR mSurface;

	VkPhysicalDevice mGpu = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties mGpuProperties;

	VkPhysicalDeviceFeatures mGpuFeatures;

	VkDevice mDevice = VK_NULL_HANDLE;

	VkQueue mGraphicsQueue;

	VkSurfaceCapabilitiesKHR mSurfaceCapabilities;

	VkSurfaceFormatKHR mSurfaceFormat;

	VkPresentModeKHR mPresentMode;

	VkExtent2D mSwapchainExtent;

	VkSwapchainKHR mSwapchain;

	uint32_t mSwapchainImageCount = 2;

	std::vector<VkImage> mSwapchainImages;

	std::vector<VkImageView> mSwapchainImageViews;

	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	VkRenderPass mRenderPass; 

	VkCommandPool mCommandPool;

	std::vector<VkCommandBuffer> mCommandBuffers;

	VkPipelineLayout mPipelineLayout;
	
	VkPipeline mPipeline;
};
