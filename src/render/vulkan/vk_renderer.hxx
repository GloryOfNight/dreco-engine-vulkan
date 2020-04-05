#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

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

	VkExtent2D mExtent2D;

	VkSwapchainKHR mSwapchain;
};
