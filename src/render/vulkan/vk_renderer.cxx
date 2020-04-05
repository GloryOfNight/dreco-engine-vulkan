#include "vk_renderer.hxx"

#include "vk_check.hxx"
#include "vk_log.hxx"
#include "vk_queue_family.hxx"
#include "vk_swapchain.hxx"

#include <vulkan/vulkan_core.h>

#include <set>
#include <iostream>

vk_renderer::vk_renderer(engine* eng) : _engine(eng), mAllocator(nullptr)
{
	createWindow();
	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
}

vk_renderer::~vk_renderer()
{
	vkFreeCommandBuffers(mDevice, mCommandPool,
		static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
	vkDestroyCommandPool(mDevice, mCommandPool, mAllocator);
	vkDestroyRenderPass(mDevice, mRenderPass, mAllocator);
	for (auto i : mSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(mDevice, i, mAllocator);
	}
	for (auto i : mSwapchainImageViews)
	{
		vkDestroyImageView(mDevice, i, mAllocator);
	}

	vkDestroySwapchainKHR(mDevice, mSwapchain, mAllocator);
	vkDestroyDevice(mDevice, mAllocator);
	glfwDestroyWindow(window);
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
    
    uint32_t extCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);

    VkInstanceCreateInfo instance_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .enabledExtensionCount = extCount,
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
	vk_queue_family queueFamily;
	queueFamily.findQueueFamilies(mGpu, mSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<uint32_t> uniqueQueueFamilies{queueFamily.mIdxGraphicsFamily};

	for (auto i : uniqueQueueFamilies)
	{
		float priorities[]{1.0f};

		VkDeviceQueueCreateInfo deviceQueueInfo{};
		deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueInfo.queueFamilyIndex = i;
		deviceQueueInfo.queueCount = 1;
		deviceQueueInfo.pQueuePriorities = priorities;
		queueCreateInfoList.push_back(deviceQueueInfo);
	}

	const uint32_t deviceExtensionsCount = 1;
	const char* deviceExtensions[deviceExtensionsCount]{"VK_KHR_swapchain"};

	// clang-format off
	VkDeviceCreateInfo deviceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size()),
		.pQueueCreateInfos = queueCreateInfoList.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = deviceExtensionsCount,
		.ppEnabledExtensionNames = deviceExtensions,
		.pEnabledFeatures = &mGpuFeatures
    };
	// clang-format on

	vk_checkError(vkCreateDevice(mGpu, &deviceCreateInfo, mAllocator, &mDevice));

	vkGetDeviceQueue(mDevice, queueFamily.mIdxGraphicsFamily, 0, &mGraphicsQueue);
}

void vk_renderer::createSwapchain()
{
	vk_swapchain swapchain{mGpu, mSurface};

	mSurfaceCapabilities = swapchain.mCapabilities;
	mSurfaceFormat = swapchain.getSurfaceFormat();
	mPresentMode = swapchain.getPresentMode();
	mSwapchainExtent = swapchain.mCapabilities.currentExtent;

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
	swapchainCreateInfo.imageExtent = mSwapchainExtent;
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

void vk_renderer::createImageViews()
{
	mSwapchainImageViews.resize(mSwapchainImageCount);

	for (uint32_t i = 0; i < mSwapchainImageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 1;
		imageViewCreateInfo.subresourceRange.layerCount = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		vk_checkError(vkCreateImageView(
			mDevice, &imageViewCreateInfo, mAllocator, &mSwapchainImageViews[i]));
	}
}

void vk_renderer::createRenderPass() 
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mSurfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass{};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subPass;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	vk_checkError(vkCreateRenderPass(mDevice, &renderPassCreateInfo, mAllocator, &mRenderPass));
}

void vk_renderer::createFramebuffers()
{
	mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

	for (uint32_t i = 0; i < mSwapchainImageViews.size(); ++i)
	{
		VkImageView attachments[]{mSwapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = mRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = mSwapchainExtent.width;
		framebufferCreateInfo.height = mSwapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		vk_checkError(vkCreateFramebuffer(
			mDevice, &framebufferCreateInfo, mAllocator, &mSwapchainFramebuffers[i]));
	}
}

void vk_renderer::createCommandPool()
{
	vk_queue_family queueFamily;
	queueFamily.findQueueFamilies(mGpu, mSurface);

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamily.mIdxGraphicsFamily;
	commandPoolCreateInfo.flags = 0;

	vk_checkError(
		vkCreateCommandPool(mDevice, &commandPoolCreateInfo, mAllocator, &mCommandPool));
}

void vk_renderer::createCommandBuffers()
{
	mCommandBuffers.resize(mSwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

	vk_checkError(vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()));
}