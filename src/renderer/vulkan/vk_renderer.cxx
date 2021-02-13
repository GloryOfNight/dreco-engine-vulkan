#include "vk_renderer.hxx"

#include "core/platform.h"
#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_allocator.hxx"
#include "vk_mesh.hxx"
#include "vk_queue_family.hxx"
#include "vk_utils.hxx"

#include <SDL_vulkan.h>
#include <iostream>
#include <stdexcept>

#define VK_USE_DEBUG 1

#if VK_USE_DEBUG
#define VK_ENABLE_VALIDATION
#define VK_ENABLE_LUNAR_MONITOR
#define VK_ENABLE_MESA_OVERLAY
#endif

vk_renderer::vk_renderer()
	: _apiVersion{0}
	, _meshes{}
	, _surface()
	, _physicalDevice()
	, _queueFamily()
	, _device()
	, _vkInstance{VK_NULL_HANDLE}
	, _vkSwapchain{VK_NULL_HANDLE}
	, _vkSwapchainImageViews{VK_NULL_HANDLE}
	, _vkFramebuffers{}
	, _vkRenderPass{VK_NULL_HANDLE}
	, _vkGraphicsCommandPool{VK_NULL_HANDLE}
	, _vkTransferCommandPool{VK_NULL_HANDLE}
	, _vkGraphicsPrimaryCommandBuffers{}
	, _vkGraphicsSecondaryCommandBuffers{}
	, _vkSubmitQueueFences{}
	, _vkSepaphoreImageAvaible{}
	, _vkSepaphoreRenderFinished{}
{
}

vk_renderer::~vk_renderer()
{
	_device.waitIdle();

	for (auto& fence : _vkSubmitQueueFences)
	{
		vkDestroyFence(_device.get(), fence, vkGetAllocator());
	}

	cleanupSwapchain(_vkSwapchain);

	for (vk_mesh* mesh : _meshes)
	{
		delete mesh;
	}
	_meshes.clear();

	vkDestroySemaphore(_device.get(), _vkSepaphoreImageAvaible, vkGetAllocator());
	vkDestroySemaphore(_device.get(), _vkSepaphoreRenderFinished, vkGetAllocator());
	vkDestroyCommandPool(_device.get(), _vkGraphicsCommandPool, vkGetAllocator());
	vkDestroyCommandPool(_device.get(), _vkTransferCommandPool, vkGetAllocator());

	_device.destroy();
	_surface.destroy(_vkInstance);

	vkDestroyInstance(_vkInstance, vkGetAllocator());

	SDL_DestroyWindow(_window);
}

vk_renderer* vk_renderer::get()
{
	if (auto eng = engine::get())
	{
		return eng->getRenderer();
	}
	return nullptr;
}

bool vk_renderer::isSupported()
{
	return NULL != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void vk_renderer::init()
{
	vkEnumerateInstanceVersion(&_apiVersion);
	createWindow();
	createInstance();
	_surface.create(_vkInstance, _window);
	_physicalDevice.setup(_vkInstance, _surface.get());
	_surface.setup(_physicalDevice.get());
	_queueFamily.setup(_physicalDevice.get(), _surface.get());
	_device.create(_physicalDevice, _queueFamily);

	createSwapchain();
	createImageViews();
	createRenderPass();
	createFramebuffers();

	createCommandPool();

	createPrimaryCommandBuffers();
	createFences();
	createSemaphores();
}

void vk_renderer::tick(double deltaTime)
{
	for (auto& mesh : _meshes)
	{
		const double rotSpeed = 1.F;
		transform mesh_transform(mesh->_transform);
		vec3 rotation(mesh_transform._rotation);
		rotation._x += rotSpeed * deltaTime;
		rotation._y += rotSpeed * deltaTime;
		rotation._z += rotSpeed * deltaTime;

		mesh_transform._rotation = rotation;

		mesh->_transform = mesh_transform;
		mesh->beforeSubmitUpdate(0);
	}
	drawFrame();
}

vk_mesh* vk_renderer::createMesh()
{
	_meshes.push_back(new vk_mesh());

	vk_mesh* newMesh = _meshes.back();
	newMesh->create();

	return newMesh;
}

uint32_t vk_renderer::getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch)
{
	major = VK_VERSION_MAJOR(_apiVersion);
	minor = VK_VERSION_MINOR(_apiVersion);
	if (nullptr != patch)
	{
		*patch = VK_VERSION_PATCH(_apiVersion);
	}
	return _apiVersion;
}

uint32_t vk_renderer::getImageCount() const
{
	return _vkSwapchainImageViews.size();
}

VkRenderPass vk_renderer::getRenderPass() const
{
	return _vkRenderPass;
}

VkCommandPool vk_renderer::getTransferCommandPool() const
{
	return _vkTransferCommandPool;
}

SDL_Window* vk_renderer::getWindow() const
{
	return _window;
}

VkAllocationCallbacks* vk_renderer::getAllocator() const
{
	return vkGetAllocator();
}

vk_device& vk_renderer::getDevice()
{
	return _device;
}

vk_surface& vk_renderer::getSurface()
{
	return _surface;
}

vk_physical_device& vk_renderer::getPhysicalDevice()
{
	return _physicalDevice;
}

vk_queue_family& vk_renderer::getQueueFamily()
{
	return _queueFamily;
}

VkCommandBuffer vk_renderer::beginSingleTimeGraphicsCommands()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = _vkGraphicsCommandPool;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device.get(), &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	return commandBuffer;
}

void vk_renderer::endSingleTimeGraphicsCommands(const VkCommandBuffer vkCommandBuffer)
{
	vkEndCommandBuffer(vkCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	vkQueueSubmit(_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_device.getGraphicsQueue());
	vkFreeCommandBuffers(_device.get(), _vkGraphicsCommandPool, 1, &vkCommandBuffer);
}

VkCommandBuffer vk_renderer::beginSingleTimeTransferCommands()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = _vkTransferCommandPool;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device.get(), &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	return commandBuffer;
}

void vk_renderer::endSingleTimeTransferCommands(const VkCommandBuffer vkCommandBuffer)
{
	vkEndCommandBuffer(vkCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	vkQueueSubmit(_device.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_device.getTransferQueue());
	vkFreeCommandBuffers(_device.get(), _vkTransferCommandPool, 1, &vkCommandBuffer);
}

void vk_renderer::createWindow()
{
	_window = SDL_CreateWindow("dreco-test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void vk_renderer::createInstance()
{
	std::vector<const char*> instExtensions;

	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(_window, &count, nullptr);
	instExtensions.resize(count);
	SDL_Vulkan_GetInstanceExtensions(_window, &count, instExtensions.data() + 0);

	uint32_t layersCount;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
	std::vector<VkLayerProperties> avaibleLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, avaibleLayers.data());

	std::vector<const char*> instLayers{};
	for (const auto& vkLayerProperty : avaibleLayers)
	{
#define PUSH_LAYER_IF_AVAIBLE(layer)                      \
	if (vkLayerProperty.layerName == std::string(#layer)) \
		instLayers.push_back(#layer);

#ifdef VK_ENABLE_VALIDATION
		PUSH_LAYER_IF_AVAIBLE(VK_LAYER_KHRONOS_validation);
#endif

#ifdef VK_ENABLE_MESA_OVERLAY
		PUSH_LAYER_IF_AVAIBLE(VK_LAYER_MESA_overlay);
#endif

#ifdef VK_ENABLE_LUNAR_MONITOR
		PUSH_LAYER_IF_AVAIBLE(VK_LAYER_LUNARG_monitor);
#endif
	}

	// clang-format off
	const VkApplicationInfo app_info
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"dreco-test",
		0,
		"dreco",
		0,
		_apiVersion
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

	VK_CHECK(vkCreateInstance(&instance_info, vkGetAllocator(), &_vkInstance));
}

void vk_renderer::createSwapchain()
{
	const VkSharingMode sharingMode{_queueFamily.getSharingMode()};

	const std::vector<uint32_t> queueFamilyIndexes =
		VK_SHARING_MODE_CONCURRENT == sharingMode ? _queueFamily.getUniqueQueueIndexes() : _queueFamily.getQueueIndexes();

	const VkSurfaceFormatKHR& surfaceFormat{_surface.getFormat()};
	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{_surface.getCapabilities()};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = _surface.get();
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = sharingMode;
	swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndexes.size());
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes.data();
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = _surface.getPresentMode();
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = _vkSwapchain;

	VK_CHECK(vkCreateSwapchainKHR(_device.get(), &swapchainCreateInfo, vkGetAllocator(), &_vkSwapchain));

	if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void vk_renderer::createImageViews()
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(_device.get(), _vkSwapchain, &imageCount, nullptr);

	std::vector<VkImage> mSwapchainImages;
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device.get(), _vkSwapchain, &imageCount, mSwapchainImages.data());

	_vkSwapchainImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _surface.getFormat().format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		VK_CHECK(vkCreateImageView(_device.get(), &imageViewCreateInfo, vkGetAllocator(), &_vkSwapchainImageViews[i]));
	}
}

void vk_renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _surface.getFormat().format;
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

	VK_CHECK(vkCreateRenderPass(_device.get(), &renderPassCreateInfo, vkGetAllocator(), &_vkRenderPass));
}

void vk_renderer::createFramebuffers()
{
	_vkFramebuffers.resize(_vkSwapchainImageViews.size());

	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{_surface.getCapabilities()};

	for (size_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
	{
		VkImageView attachments{_vkSwapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = _vkRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &attachments;
		framebufferCreateInfo.width = surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height = surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(_device.get(), &framebufferCreateInfo, vkGetAllocator(), &_vkFramebuffers[i]));
	}
}

void vk_renderer::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = _queueFamily.getGraphicsIndex();
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK(vkCreateCommandPool(_device.get(), &commandPoolCreateInfo, vkGetAllocator(), &_vkGraphicsCommandPool));

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = _queueFamily.getTransferIndex();
	commandPoolCreateInfo.flags = 0;

	VK_CHECK(vkCreateCommandPool(_device.get(), &commandPoolCreateInfo, vkGetAllocator(), &_vkTransferCommandPool));
}

void vk_renderer::createPrimaryCommandBuffers()
{
	_vkGraphicsPrimaryCommandBuffers.resize(_vkSwapchainImageViews.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _vkGraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(_vkGraphicsPrimaryCommandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(_device.get(), &allocInfo, _vkGraphicsPrimaryCommandBuffers.data()));
}

VkCommandBuffer vk_renderer::createSecondaryCommandBuffer()
{
	_vkGraphicsSecondaryCommandBuffers.push_back(VK_NULL_HANDLE);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _vkGraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(_device.get(), &allocInfo, &_vkGraphicsSecondaryCommandBuffers.back()));

	return _vkGraphicsSecondaryCommandBuffers.back();
}

inline void vk_renderer::createFences()
{
	const size_t size = _vkSwapchainImageViews.size();
	_vkSubmitQueueFences.resize(size);
	for (auto& fence : _vkSubmitQueueFences)
	{
		VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
		VK_CHECK(vkCreateFence(_device.get(), &createInfo, vkGetAllocator(), &fence));
	}
}

void vk_renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK(vkCreateSemaphore(_device.get(), &semaphoreInfo, vkGetAllocator(), &_vkSepaphoreImageAvaible));
	VK_CHECK(vkCreateSemaphore(_device.get(), &semaphoreInfo, vkGetAllocator(), &_vkSepaphoreRenderFinished));
}

void vk_renderer::drawFrame()
{
	uint32_t imageIndex;
	if (const VkResult result = vkAcquireNextImageKHR(_device.get(), _vkSwapchain, UINT32_MAX, _vkSepaphoreImageAvaible, VK_NULL_HANDLE, &imageIndex);
		VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			recreateSwapchain();
		}
		else
		{
			VK_RETURN_ON_RESULT(result, VK_TIMEOUT);
		}
		return;
	}

	const VkResult result = vkWaitForFences(_device.get(), 1, &_vkSubmitQueueFences[imageIndex], true, UINT32_MAX);
	VK_RETURN_ON_RESULT(result, VK_TIMEOUT);
	VK_CHECK(result);

	vkResetFences(_device.get(), 1, &_vkSubmitQueueFences[imageIndex]);

	prepareCommandBuffer(imageIndex);

	std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	std::array<VkSemaphore, 1> waitSemaphores = {_vkSepaphoreImageAvaible};
	std::array<VkSemaphore, 1> signalSemaphores = {_vkSepaphoreRenderFinished};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_vkGraphicsPrimaryCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	VK_CHECK(vkQueueSubmit(_device.getGraphicsQueue(), 1, &submitInfo, _vkSubmitQueueFences[imageIndex]));

	std::array<VkSwapchainKHR, 1> swapchains{_vkSwapchain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(_device.getPresentQueue(), &presentInfo);
}

void vk_renderer::cleanupSwapchain(VkSwapchainKHR& swapchain)
{
	_device.waitIdle();

	vkDestroyRenderPass(_device.get(), _vkRenderPass, vkGetAllocator());

	for (auto frameBuffer : _vkFramebuffers)
	{
		vkDestroyFramebuffer(_device.get(), frameBuffer, vkGetAllocator());
	}
	_vkFramebuffers.clear();

	for (auto imageView : _vkSwapchainImageViews)
	{
		vkDestroyImageView(_device.get(), imageView, vkGetAllocator());
	}
	_vkSwapchainImageViews.clear();

	vkDestroySwapchainKHR(_device.get(), swapchain, vkGetAllocator());
}

void vk_renderer::recreateSwapchain()
{
	_surface.setup(_physicalDevice.get());

	const VkExtent2D currentExtent{_surface.getCapabilities().currentExtent};
	if (0 == currentExtent.height || 0 == currentExtent.width)
	{
		return;
	}

	createSwapchain();
	createImageViews();

	createRenderPass();
	createFramebuffers();

	for (vk_mesh* mesh : _meshes)
	{
		mesh->recreatePipeline(_vkRenderPass, currentExtent);
	}
}

void vk_renderer::prepareCommandBuffer(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = _vkGraphicsPrimaryCommandBuffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	const VkClearValue clearColor{{{0.0F, 0.0F, 0.0F, 1.0F}}};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _vkRenderPass;
	renderPassInfo.framebuffer = _vkFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _surface.getCapabilities().currentExtent;
	renderPassInfo.clearValueCount = 4;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	const uint32_t graphicsCommandBuffersSize{static_cast<uint32_t>(_vkGraphicsSecondaryCommandBuffers.size())};
	if (graphicsCommandBuffersSize > 0)
	{
		vkCmdExecuteCommands(commandBuffer, _vkGraphicsSecondaryCommandBuffers.size(), _vkGraphicsSecondaryCommandBuffers.data());
	}
	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}