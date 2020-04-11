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
	
	void drawFrame();

protected:
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

	inline void cleanupSwapchain();

	inline void recreateSwapchain();
private:
	engine* _engine;

	GLFWwindow* window;

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

	VkPresentModeKHR mPresentMode;

	VkExtent2D mSwapchainExtent;
	VkSwapchainKHR mSwapchain;
	uint32_t mSwapchainImageCount;
	std::vector<VkImage> mSwapchainImages;
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
