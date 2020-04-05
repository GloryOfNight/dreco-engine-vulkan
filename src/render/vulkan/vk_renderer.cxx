#include "vk_renderer.hxx"

#include "vk_check.hxx"
#include "vk_log.hxx"
#include "vk_queue_family.hxx"
#include "vk_swapchain.hxx"

#include <vulkan/vulkan_core.h>
#include <iostream>

vk_renderer::vk_renderer(engine* eng) : _engine(eng), mAllocator(nullptr)
{
	createWindow();
	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
}

vk_renderer::~vk_renderer()
{
	glfwDestroyWindow(window);
	vkDestroySwapchainKHR(mDevice, mSwapchain, mAllocator);
	vkDestroyDevice(mDevice, mAllocator);
	vkDestroySurfaceKHR(mInstance, mSurface, mAllocator);
	vkDestroyInstance(mInstance, nullptr);
}

void vk_renderer::tick(const float& delta_time)
{
}

GLFWwindow* vk_renderer::getWindow() const
{
	return window;
}

bool vk_renderer::isSupported()
{
	return glfwVulkanSupported();
}

void vk_renderer::createWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(540, 960, "dreco-test", nullptr, nullptr);
}

void vk_renderer::createInstance()
{
	// clang-format off
    VkApplicationInfo app_info
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "dreco-test",
        .applicationVersion = 0,
        .pEngineName = "dreco-engine",
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_1
    };
    
    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    VkInstanceCreateInfo instance_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .enabledExtensionCount = count,
        .ppEnabledExtensionNames = extensions
    };
	// clang-format on

	vk_checkError(vkCreateInstance(&instance_info, mAllocator, &mInstance));
}

void vk_renderer::createSurface()
{
	vk_checkError(glfwCreateWindowSurface(mInstance, window, mAllocator, &mSurface));
}

void vk_renderer::selectPhysicalDevice()
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr);
	VkPhysicalDevice gpuList[gpuCount];
	vkEnumeratePhysicalDevices(mInstance, &gpuCount, &gpuList[0]);

	for (uint32_t i = 0; i < gpuCount; ++i)
	{
		if (glfwGetPhysicalDevicePresentationSupport(mInstance, gpuList[i], i))
		{
			mGpu = gpuList[i];
			vkGetPhysicalDeviceProperties(mGpu, &mGpuProperties);
			vkGetPhysicalDeviceFeatures(mGpu, &mGpuFeatures);
			break;
		}
	}

	if (VK_NULL_HANDLE == mGpu)
	{
		std::cerr << "No supported GPU found!" << std::endl;
	}
}

void vk_renderer::createLogicalDevice()
{
	uint32_t queueFamilyPropertiesCount;
	vkGetPhysicalDeviceQueueFamilyProperties(mGpu, &queueFamilyPropertiesCount, nullptr);
	VkQueueFamilyProperties queueFamilyProperties[queueFamilyPropertiesCount];
	vkGetPhysicalDeviceQueueFamilyProperties(
		mGpu, &queueFamilyPropertiesCount, &queueFamilyProperties[0]);

	VkDeviceQueueCreateInfo deviceQueueInfos[queueFamilyPropertiesCount]{};
	for (uint32_t i = 0; i < queueFamilyPropertiesCount; ++i)
	{
		float priorities[]{1.0f};

		VkDeviceQueueCreateInfo& deviceQueueInfo = deviceQueueInfos[0];
		deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueInfo.queueFamilyIndex = i;
		deviceQueueInfo.queueCount = queueFamilyPropertiesCount;
		deviceQueueInfo.pQueuePriorities = priorities;
	}

	// clang-format off
	VkDeviceCreateInfo deviceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.queueCreateInfoCount = queueFamilyPropertiesCount,
		.pQueueCreateInfos = deviceQueueInfos,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &mGpuFeatures
    };
	// clang-format on

	vk_checkError(vkCreateDevice(mGpu, &deviceCreateInfo, mAllocator, &mDevice));

	vkGetDeviceQueue(mDevice, 0, 0, &mGraphicsQueue);
}

void vk_renderer::createSwapchain()
{
	vk_swapchain swapchain{mGpu, mSurface};

	mSurfaceCapabilities = swapchain.mCapabilities;
	mSurfaceFormat = swapchain.getSurfaceFormat();
	mPresentMode = swapchain.getPresentMode();
	mExtent2D = swapchain.mCapabilities.currentExtent;

	const uint32_t mSwapchainImageCount = mSurfaceCapabilities.minImageCount + 1;

	vk_queue_family queueFamily;
	queueFamily.findQueueFamilies(mGpu, mSurface);
	uint32_t queueFamilyIndexes[2]{
		queueFamily.mIdxGraphicsFamily, queueFamily.mIdxPresentFamily};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = mSurface;
	swapchainCreateInfo.minImageCount = mSwapchainImageCount;
	swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = mExtent2D;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.queueFamilyIndexCount = 2;
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes;
	swapchainCreateInfo.preTransform = mSurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = mPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	vk_checkError(
		vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, mAllocator, &mSwapchain));
}
