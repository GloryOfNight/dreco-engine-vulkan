#include "vk_renderer.hxx"

#include "vk_check.hxx"
#include "vk_log.hxx"

#include <iostream>

vk_renderer::vk_renderer(engine* eng) : _engine(eng), allocator(nullptr)
{
	createWindow();
	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
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
        .pNext = nullptr,
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

	vk_checkError(vkCreateInstance(&instance_info, allocator, &instance));
	if (GL_FALSE == glfwVulkanSupported())
	{
		std::cerr << "Vulkan not supported on that device!" << std::endl;
	}
}

void vk_renderer::createSurface()
{
	vk_checkError(glfwCreateWindowSurface(instance, window, allocator, &surface));
}

void vk_renderer::selectPhysicalDevice()
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
	VkPhysicalDevice gpuList[gpuCount];
	vkEnumeratePhysicalDevices(instance, &gpuCount, &gpuList[0]);

	for (uint32_t i = 0; i < gpuCount; ++i)
	{
		if (glfwGetPhysicalDevicePresentationSupport(instance, gpuList[i], i))
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
		.queueCreateInfoCount = queueFamilyPropertiesCount,
		.pQueueCreateInfos = deviceQueueInfos,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &mGpuFeatures
    };
	// clang-format on

	vk_checkError(vkCreateDevice(mGpu, &deviceCreateInfo, allocator, &device));

	vkGetDeviceQueue(device, 0, 0, &mGraphicsQueue);
}

vk_renderer::~vk_renderer()
{
	glfwDestroyWindow(window);
	vkDestroyDevice(device, allocator);
	vkDestroySurfaceKHR(instance, surface, allocator);
	vkDestroyInstance(instance, nullptr);
}

GLFWwindow* vk_renderer::getWindow() const
{
	return window;
}

void vk_renderer::tick(const float& delta_time)
{
}