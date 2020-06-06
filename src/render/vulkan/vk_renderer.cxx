#include "vk_renderer.hxx"
#include "vk_check.hxx"
#include "vk_log.hxx"
#include "vk_queue_family.hxx"
#include "vk_swapchain.hxx"
#include "core/utils/file_utils.hxx"

#include "SDL2/SDL_vulkan.h"
#include <vulkan/vulkan_core.h>
#include <set>
#include <iostream>
#include <stdexcept>

#define VK_USE_VALIDATION

vk_renderer::vk_renderer(engine* eng) : _engine(eng), mAllocator(nullptr)
{
	createWindow();
	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();

	createSemaphores();
	createCommandPool();

	setupSurfaceCapabilities();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createCommandBuffers();
	createPipelineLayout();
	createGraphicsPipeline();
	recordCommandBuffers();
}

vk_renderer::~vk_renderer()
{
	vkDeviceWaitIdle(mDevice);

	cleanupSwapchain(mSwapchain);

	vkDestroySemaphore(mDevice, mSepaphore_Image_Avaible, mAllocator);
	vkDestroySemaphore(mDevice, mSepaphore_Render_Finished, mAllocator);
	vkDestroyCommandPool(mDevice, mCommandPool, mAllocator);

	vkDestroyDevice(mDevice, mAllocator);
	SDL_DestroyWindow(window);
	vkDestroySurfaceKHR(mInstance, mSurface, mAllocator);
	vkDestroyInstance(mInstance, nullptr);
}

void vk_renderer::tick(const float& delta_time)
{
	drawFrame();
}

SDL_Window* vk_renderer::getWindow() const
{
	return window;
}

bool vk_renderer::isSupported()
{
	return true;
}

void vk_renderer::createWindow()
{
	window = SDL_CreateWindow("dreco-test", 0, 0, 720, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void vk_renderer::createInstance()
{
	std::vector<const char*> instExtensions{"VK_KHR_surface", "VK_KHR_xlib_surface"};
#ifdef VK_USE_VALIDATION
	instExtensions.push_back("VK_EXT_debug_utils");
#endif

	std::vector<const char*> instLayers{};
#ifdef VK_USE_VALIDATION
	instLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	// clang-format off
	const VkApplicationInfo app_info
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"dreco-test",
		0,
		"dreco",
		0,
		VK_API_VERSION_1_1
	};

	const VkInstanceCreateInfo instance_info
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&app_info,
		static_cast<uint32_t>(instLayers.size()),
		instLayers.data(),
		static_cast<uint32_t>(instExtensions.size()),
		instExtensions.data()
	};
	// clang-format on

	vk_checkError(vkCreateInstance(&instance_info, mAllocator, &mInstance));
}

void vk_renderer::createSurface()
{
	SDL_Vulkan_CreateSurface(window, mInstance, &mSurface);
}

void vk_renderer::selectPhysicalDevice()
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr);
	VkPhysicalDevice gpuList[gpuCount];
	vkEnumeratePhysicalDevices(mInstance, &gpuCount, &gpuList[0]);

	for (auto& gpu : gpuList)
	{	
		//uint32_t count;
		//vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
		//VkQueueFamilyProperties properties[count];
		//vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, &properties[0]);
		// TODO: Need to find supported queue (if there one), not only 0
		VkBool32 isSupported;
		vk_checkError(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, 0, mSurface, &isSupported));

		if (isSupported)
		{
			mGpu = gpu;
			vkGetPhysicalDeviceProperties(mGpu, &mGpuProperties);
			vkGetPhysicalDeviceFeatures(mGpu, &mGpuFeatures);
			break;
		}
	}

	if (VK_NULL_HANDLE == mGpu)
	{
		throw std::runtime_error("No supported GPU found!");
	}
}

void vk_renderer::createLogicalDevice()
{
	vk_queue_family queueFamily;
	queueFamily.findQueueFamilies(mGpu, mSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<uint32_t> uniqueQueueFamilies{queueFamily.mIdxGraphicsFamily};

	float priorities[]{1.0f};

	for (auto i : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo deviceQueueInfo{};
		deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueInfo.queueFamilyIndex = i;
		deviceQueueInfo.queueCount = 1;
		deviceQueueInfo.pQueuePriorities = priorities;
		queueCreateInfoList.push_back(deviceQueueInfo);
	}

	const uint32_t deviceExtensionsCount = 1;
	const char* deviceExtensions[deviceExtensionsCount]{"VK_KHR_swapchain"};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = deviceExtensionsCount;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.pEnabledFeatures = &mGpuFeatures;

	vk_checkError(vkCreateDevice(mGpu, &deviceCreateInfo, mAllocator, &mDevice));

	vkGetDeviceQueue(mDevice, queueFamily.mIdxGraphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, queueFamily.mIdxPresentFamily, 0, &mPresentQueue);
}

void vk_renderer::setupSurfaceCapabilities()
{
	vk_checkError(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mGpu, mSurface, &mSurfaceCapabilities));
}

void vk_renderer::createSwapchain()
{
	vk_swapchain swapchain{mGpu, mSurface};

	mSurfaceFormat = swapchain.getSurfaceFormat();

	vk_queue_family queueFamily;
	queueFamily.findQueueFamilies(mGpu, mSurface);
	// TODO: if indexex the same, no need to put both in here
	// if different, imageSharingMode need to be VK_SHARING_MODE_CONCURRENT
	uint32_t queueFamilyIndexes[2]{queueFamily.mIdxGraphicsFamily, queueFamily.mIdxPresentFamily};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = mSurface;
	swapchainCreateInfo.minImageCount = mSurfaceCapabilities.minImageCount;
	swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = mSurfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 2;
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes;
	swapchainCreateInfo.preTransform = mSurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = swapchain.getPresentMode();
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = mSwapchain;

	vk_checkError(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, mAllocator, &mSwapchain));

	if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void vk_renderer::createImageViews()
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);

	std::vector<VkImage> mSwapchainImages;
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
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
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		vk_checkError(vkCreateImageView(mDevice, &imageViewCreateInfo, mAllocator, &mSwapchainImageViews[i]));
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

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency subpassDependecy{};
	subpassDependecy.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependecy.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependecy.srcAccessMask = 0;
	subpassDependecy.dstSubpass = 0;
	subpassDependecy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependecy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependecy;

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
		framebufferCreateInfo.width = mSurfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height = mSurfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		vk_checkError(vkCreateFramebuffer(mDevice, &framebufferCreateInfo, mAllocator, &mSwapchainFramebuffers[i]));
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

	vk_checkError(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, mAllocator, &mCommandPool));
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

void vk_renderer::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;
	vk_checkError(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, mAllocator, &mPipelineLayout));
}

void vk_renderer::createGraphicsPipeline()
{
	size_t vertShaderSize;
	char* vertShaderCode = file_utils::read_file("shaders/vert.spv", &vertShaderSize);
	if (nullptr == vertShaderCode)
		throw std::runtime_error("Failed to load binary shader code");
	size_t fragShaderSize;
	char* fragShaderCode = file_utils::read_file("shaders/frag.spv", &fragShaderSize);
	if (nullptr == fragShaderCode)
		throw std::runtime_error("Failed to load binary shader code");

	VkShaderModule vertShaderModule;
	createShaderModule(vertShaderCode, vertShaderSize, vertShaderModule);
	VkShaderModule fragShaderModule;
	createShaderModule(fragShaderCode, fragShaderSize, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStagesInfo[]{vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(mSurfaceCapabilities.currentExtent.width);
	viewport.height = static_cast<float>(mSurfaceCapabilities.currentExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors{};
	scissors.offset = {0, 0};
	scissors.extent = mSurfaceCapabilities.currentExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStagesInfo;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	vk_checkError(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, mAllocator, &mPipeline));

	vkDestroyShaderModule(mDevice, vertShaderModule, mAllocator);
	vkDestroyShaderModule(mDevice, fragShaderModule, mAllocator);
	delete[] vertShaderCode;
	delete[] fragShaderCode;
}

void vk_renderer::createShaderModule(const char* src, const size_t& src_size, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = src_size;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(src);

	vk_checkError(vkCreateShaderModule(mDevice, &shaderModuleCreateInfo, mAllocator, &shaderModule));
}

void vk_renderer::recordCommandBuffers()
{
	for (size_t i = 0; i < mCommandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vk_checkError(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo));

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = mSurfaceCapabilities.currentExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
		vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(mCommandBuffers[i]);

		vk_checkError(vkEndCommandBuffer(mCommandBuffers[i]));
	}
}

void vk_renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vk_checkError(vkCreateSemaphore(mDevice, &semaphoreInfo, mAllocator, &mSepaphore_Image_Avaible));
	vk_checkError(vkCreateSemaphore(mDevice, &semaphoreInfo, mAllocator, &mSepaphore_Render_Finished));
}

void vk_renderer::drawFrame()
{
	uint32_t imageIndex;
	if (VkResult result = vkAcquireNextImageKHR(
			mDevice, mSwapchain, UINT32_MAX, mSepaphore_Image_Avaible, VK_NULL_HANDLE, &imageIndex);
		VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			recreateSwapchain();
		}
		return;
	}

	VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore waitSemaphores[]{mSepaphore_Image_Avaible};
	VkSemaphore signalSemaphores[]{mSepaphore_Render_Finished};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vk_checkError(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	VkSwapchainKHR swapchains[]{mSwapchain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(mPresentQueue, &presentInfo);
}

void vk_renderer::cleanupSwapchain(VkSwapchainKHR& swapchain)
{
	vkDeviceWaitIdle(mDevice);

	vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
	mCommandBuffers.clear();

	vkDestroyPipeline(mDevice, mPipeline, mAllocator);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, mAllocator);
	vkDestroyRenderPass(mDevice, mRenderPass, mAllocator);

	for (auto frameBuffer : mSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(mDevice, frameBuffer, mAllocator);
	}
	mSwapchainFramebuffers.clear();

	for (auto imageView : mSwapchainImageViews)
	{
		vkDestroyImageView(mDevice, imageView, mAllocator);
	}
	mSwapchainImageViews.clear();

	vkDestroySwapchainKHR(mDevice, swapchain, mAllocator);
}

void vk_renderer::recreateSwapchain()
{
	setupSurfaceCapabilities();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createCommandBuffers();
	createPipelineLayout();
	createGraphicsPipeline();
	recordCommandBuffers();
}