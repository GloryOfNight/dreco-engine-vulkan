#include "vk_renderer.hxx"
#include "vk_check.hxx"
#include "vk_log.hxx"
#include "vk_queue_family.hxx"
#include "vk_swapchain.hxx"
#include "core/utils/file_utils.hxx"
#include "core/platform.h"
#include "engine/engine.hxx"

#include <SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include <set>
#include <iostream>
#include <stdexcept>

#define VK_ENABLE_VALIDATION
//#define VK_ENABLE_MESA_OVERLAY

vk_renderer::vk_renderer(engine* eng) : _engine(eng), mAllocator(nullptr)
{
	createWindow();
	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();

	createSemaphores();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createDescriptorSetLayout();

	recreateSwapchain();
}

vk_renderer::~vk_renderer()
{
	vkDeviceWaitIdle(mDevice);

	cleanupSwapchain(mSwapchain);
	destroyBuffer(vertexBuffer, vertexBufferMemory);
	destroyBuffer(indexBuffer, indexBufferMemory);

	vkDestroyDescriptorSetLayout(mDevice, descriptorSetLayout, mAllocator);
	vkDestroySemaphore(mDevice, mSepaphore_Image_Avaible, mAllocator);
	vkDestroySemaphore(mDevice, mSepaphore_Render_Finished, mAllocator);
	vkDestroyCommandPool(mDevice, mGraphicsCommandPool, mAllocator);
	vkDestroyCommandPool(mDevice, mTransferCommandPool, mAllocator);

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

void vk_renderer::createWindow()
{
	window = SDL_CreateWindow("dreco-test", 0, 0, 720, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void vk_renderer::createInstance()
{
	std::vector<const char*> instExtensions{"VK_KHR_surface"};

#if PLATFORM_LINUX
	instExtensions.push_back("VK_KHR_xlib_surface");
#elif PLATFORM_WINDOWS
	instExtensions.push_back("VK_KHR_win32_surface");
#endif


#ifdef VK_ENABLE_VALIDATION
	instExtensions.push_back("VK_EXT_debug_utils");
#endif

	std::vector<const char*> instLayers{};
#ifdef VK_ENABLE_VALIDATION
	instLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

#ifdef VK_ENABLE_MESA_OVERLAY
	instLayers.push_back("VK_LAYER_MESA_overlay");
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
	std::vector<VkPhysicalDevice> gpuList(gpuCount);
	vkEnumeratePhysicalDevices(mInstance, &gpuCount, gpuList.data());

	for (auto& gpu : gpuList)
	{
		queueFamily = vk_queue_family(gpu, mSurface);
		if (true == queueFamily.isSupported)
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
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<uint32_t> uniqueQueueFamilies{queueFamily.graphicsQueueFamilyIndex, queueFamily.presentQueueFamilyIndex,
		queueFamily.transferQueueFamilyIndex};

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

	vkGetDeviceQueue(mDevice, queueFamily.graphicsQueueFamilyIndex, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, queueFamily.transferQueueFamilyIndex, 0, &mTransferQueue);
	vkGetDeviceQueue(mDevice, queueFamily.presentQueueFamilyIndex, 0, &mPresentQueue);
}

void vk_renderer::setupSurfaceCapabilities()
{
	vk_checkError(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mGpu, mSurface, &mSurfaceCapabilities));
}

void vk_renderer::createSwapchain()
{
	vk_swapchain swapchain{mGpu, mSurface};

	mSurfaceFormat = swapchain.getSurfaceFormat();

	VkSharingMode sharingMode{queueFamily.sharingMode};
	std::vector<uint32_t> queueFamilyIndexes{queueFamily.graphicsQueueFamilyIndex, queueFamily.transferQueueFamilyIndex,
		queueFamily.presentQueueFamilyIndex};

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
	swapchainCreateInfo.imageSharingMode = sharingMode;
	swapchainCreateInfo.queueFamilyIndexCount = queueFamilyIndexes.size();
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes.data();
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
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamily.graphicsQueueFamilyIndex;
	commandPoolCreateInfo.flags = 0;

	vk_checkError(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, mAllocator, &mGraphicsCommandPool));

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamily.transferQueueFamilyIndex;
	commandPoolCreateInfo.flags = 0;

	vk_checkError(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, mAllocator, &mTransferCommandPool));
}

void vk_renderer::createCommandBuffers()
{
	mGraphicsCommandBuffers.resize(mSwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mGraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(mGraphicsCommandBuffers.size());

	vk_checkError(vkAllocateCommandBuffers(mDevice, &allocInfo, mGraphicsCommandBuffers.data()));
}

void vk_renderer::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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

	// TODO: Description should be somewhere else
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(vec3);

	// TODO: Attributes should be somewhere else
	VkVertexInputAttributeDescription vertexInputAttributeDescription{};
	vertexInputAttributeDescription.binding = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;

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
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	colorBlendAttachment.blendEnable = VK_FALSE;
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
	for (size_t i = 0; i < mGraphicsCommandBuffers.size(); ++i)
	{
		VkCommandBuffer& commandBuffer = mGraphicsCommandBuffers[i];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vk_checkError(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = mSurfaceCapabilities.currentExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

		VkBuffer buffers[1]{vertexBuffer};
		VkDeviceSize offsets[1]{0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, mesh._indexes.size(), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		vk_checkError(vkEndCommandBuffer(commandBuffer));
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

	updateUniformBuffers(imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mGraphicsCommandBuffers[imageIndex];
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

	for (uint32_t i = 0; i < uniformBuffers.size(); ++i)
	{
		destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
	}

	vkDestroyDescriptorPool(mDevice, mDescriptorPool, mAllocator);

	vkFreeCommandBuffers(mDevice, mGraphicsCommandPool, static_cast<uint32_t>(mGraphicsCommandBuffers.size()),
		mGraphicsCommandBuffers.data());
	mGraphicsCommandBuffers.clear();

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

	createDescriptorPool();
	createUniformBuffers();
	createDescriptorSets();
	
	createRenderPass();
	createFramebuffers();
	createCommandBuffers();
	createPipelineLayout();
	createGraphicsPipeline();
	recordCommandBuffers();
}

void vk_renderer::updateUniformBuffers(uint32_t image)
{
	ubo._model = mat4::makeRotation(_engine->shapeTranslation) ;
	ubo._view = mat4::makeTranslation(vec3{0,0,1.3f});
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	ubo._projection = mat4::makeProjection(-1,1, static_cast<float>(w) / static_cast<float>(h), 75.f);
	//ubo._projection._mat[1][1] = -1;

	void* data;
	vkMapMemory(mDevice, uniformBuffersMemory[image], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mDevice, uniformBuffersMemory[image]);
}

void vk_renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &uboLayoutBinding;

	vk_checkError(vkCreateDescriptorSetLayout(mDevice, &layoutCreateInfo, mAllocator, &descriptorSetLayout));
}

void vk_renderer::createDescriptorSets()
{
	const uint32_t imageSize = mSwapchainImageViews.size();
	std::vector<VkDescriptorSetLayout> layouts(imageSize, descriptorSetLayout);
	mDescriptorSets.resize(imageSize);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = imageSize;
	allocInfo.pSetLayouts = layouts.data();

	vk_checkError(vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()));

	for (uint32_t i = 0; i < imageSize; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.pNext = nullptr;
		descriptorWrite.dstSet = mDescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void vk_renderer::createVertexBuffer()
{
	const VkDeviceSize bufferSize{sizeof(mesh._vertexes[0]) * mesh._vertexes.size()};

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferMemory,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
	memcpy(data, mesh._vertexes.data(), bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, vertexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, mAllocator);
	vkFreeMemory(mDevice, stagingBufferMemory, mAllocator);
}

void vk_renderer::createIndexBuffer()
{
	const VkDeviceSize bufferSize{sizeof(mesh._indexes[0]) * mesh._indexes.size()};

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferMemory,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
	memcpy(data, mesh._indexes.data(), bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, indexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		indexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, mAllocator);
	vkFreeMemory(mDevice, stagingBufferMemory, mAllocator);
}

void vk_renderer::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = mSwapchainImageViews.size();

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = mSwapchainImageViews.size();

	vk_checkError(vkCreateDescriptorPool(mDevice, &poolCreateInfo, mAllocator, &mDescriptorPool));
}

void vk_renderer::createUniformBuffers()
{
	const VkDeviceSize size = sizeof(UniformBufferObject);
	const size_t imageCount{mSwapchainImageViews.size()};

	uniformBuffers.resize(imageCount);
	uniformBuffersMemory.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		createBuffer(size, uniformBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffersMemory[i],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

void vk_renderer::createBuffer(VkDeviceSize size, VkBuffer& buffer, VkBufferUsageFlags bufferUsageFlags,
	VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags memoryPropertyFlags)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = queueFamily.sharingMode;
	if (VK_SHARING_MODE_CONCURRENT == bufferCreateInfo.sharingMode)
	{
		uint32_t queueFamilyIndexes[3] = {queueFamily.graphicsQueueFamilyIndex, queueFamily.presentQueueFamilyIndex,
			queueFamily.transferQueueFamilyIndex};

		bufferCreateInfo.queueFamilyIndexCount = 3;
		bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndexes;
	}

	vk_checkError(vkCreateBuffer(mDevice, &bufferCreateInfo, mAllocator, &buffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(mDevice, buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	vk_checkError(vkAllocateMemory(mDevice, &memoryAllocateInfo, mAllocator, &bufferMemory));

	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

void vk_renderer::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = mTransferCommandPool;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mTransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mTransferQueue);
	vkFreeCommandBuffers(mDevice, mTransferCommandPool, 1, &commandBuffer);
}

uint32_t vk_renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(mGpu, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) &&
			(memoryProperties.memoryTypes[i].propertyFlags & propertiesFlags) == propertiesFlags)
		{
			return i;
		}
	}
	throw std::runtime_error("No suitable memory find!");
	return 0;
}

void vk_renderer::destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	vkDestroyBuffer(mDevice, buffer, mAllocator);
	vkFreeMemory(mDevice, bufferMemory, mAllocator);
}