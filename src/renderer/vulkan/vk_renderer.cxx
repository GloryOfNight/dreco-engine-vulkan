#include "vk_renderer.hxx"

#include "core/platform.h"
#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_mesh.hxx"
#include "vk_queue_family.hxx"
#include "vk_utils.hxx"

#include <SDL_vulkan.h>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define VK_ENABLE_VALIDATION
//#define VK_ENABLE_MESA_OVERLAY

vk_renderer::vk_renderer(engine* eng)
	: _engine(eng)
	, _vkAllocator(nullptr)
	, _surface(&_vkInstance)
	, _physicalDevice(&_vkInstance)
	, _device()
{
	createWindow();
	createInstance();
	_surface.create(getWindow());
	_physicalDevice.setup(_surface.get());
	_surface.setup(_physicalDevice.get());
	_queueFamily.setup(_physicalDevice.get(), _surface.get());
	_device.create(_physicalDevice, _queueFamily);

	createSemaphores();
	createCommandPool();

	_surface.setup(_physicalDevice.get());

	createSwapchain();
	createImageViews();

	createCommandBuffers();

	createRenderPass();
	createFramebuffers();

	createFences();
}

vk_renderer::~vk_renderer()
{
	_device.waitIdle();

	for (auto& fence : _vkSubmitQueueFences)
	{
		vkDestroyFence(_device.get(), fence, _vkAllocator);
	}

	cleanupSwapchain(_vkSwapchain);

	for (vk_mesh* mesh : _meshes)
	{
		delete mesh;
	}
	_meshes.clear();

	vkDestroySemaphore(_device.get(), _vkSepaphoreImageAvaible, _vkAllocator);
	vkDestroySemaphore(_device.get(), _vkSepaphoreRenderFinished, _vkAllocator);
	vkDestroyCommandPool(_device.get(), _vkGraphicsCommandPool, _vkAllocator);
	vkDestroyCommandPool(_device.get(), _vkTransferCommandPool, _vkAllocator);

	_device.destroy();
	SDL_DestroyWindow(_window);
	_surface.destroy();
	vkDestroyInstance(_vkInstance, nullptr);
}

void vk_renderer::tick(const float& delta_time)
{
	drawFrame();
}

void vk_renderer::createMesh()
{
	_meshes.push_back(new vk_mesh());
	// clang-format off
	vk_mesh_create_info mesh_create_info
	{
		&_device,
		&_queueFamily,
		&_physicalDevice,
		_vkRenderPass,
		_surface.getCapabilities().currentExtent,
		static_cast<uint32_t>(_vkSwapchainImageViews.size())
	};
	// clang-format on
	_meshes.back()->create(mesh_create_info);
}

SDL_Window* vk_renderer::getWindow() const
{
	return _window;
}

VkAllocationCallbacks* vk_renderer::getAllocator() const
{
	return _vkAllocator;
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

void vk_renderer::createWindow()
{
	_window = SDL_CreateWindow("dreco-test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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

	VK_CHECK(vkCreateInstance(&instance_info, _vkAllocator, &_vkInstance));
}

void vk_renderer::createSwapchain()
{
	VkSharingMode sharingMode{_queueFamily.getSharingMode()};
	std::vector<uint32_t> queueFamilyIndexes{
		_queueFamily.getGraphicsIndex(),
		_queueFamily.getTransferIndex(),
		_queueFamily.getPresentIndex()};

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

	VK_CHECK(vkCreateSwapchainKHR(_device.get(), &swapchainCreateInfo, _vkAllocator, &_vkSwapchain));

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

		VK_CHECK(vkCreateImageView(_device.get(), &imageViewCreateInfo, _vkAllocator, &_vkSwapchainImageViews[i]));
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

	VK_CHECK(vkCreateRenderPass(_device.get(), &renderPassCreateInfo, _vkAllocator, &_vkRenderPass));
}

void vk_renderer::createFramebuffers()
{
	_vkFramebuffers.resize(_vkSwapchainImageViews.size());

	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{_surface.getCapabilities()};

	for (uint32_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
	{
		VkImageView attachments[]{_vkSwapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = _vkRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height = surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(_device.get(), &framebufferCreateInfo, _vkAllocator, &_vkFramebuffers[i]));
	}
}

void vk_renderer::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = _queueFamily.getGraphicsIndex();
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK(vkCreateCommandPool(_device.get(), &commandPoolCreateInfo, _vkAllocator, &_vkGraphicsCommandPool));

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = _queueFamily.getTransferIndex();
	commandPoolCreateInfo.flags = 0;

	VK_CHECK(vkCreateCommandPool(_device.get(), &commandPoolCreateInfo, _vkAllocator, &_vkTransferCommandPool));
}

void vk_renderer::createCommandBuffers()
{
	_vkGraphicsCommandBuffers.resize(_vkSwapchainImageViews.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _vkGraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(_vkGraphicsCommandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(_device.get(), &allocInfo, _vkGraphicsCommandBuffers.data()));
}

inline void vk_renderer::createFences()
{
	const size_t size = _vkSwapchainImageViews.size();
	_vkSubmitQueueFences.resize(size);
	for (auto& fence : _vkSubmitQueueFences)
	{
		VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
		VK_CHECK(vkCreateFence(_device.get(), &createInfo, _vkAllocator, &fence));
	}
}

void vk_renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK(vkCreateSemaphore(_device.get(), &semaphoreInfo, _vkAllocator, &_vkSepaphoreImageAvaible));
	VK_CHECK(vkCreateSemaphore(_device.get(), &semaphoreInfo, _vkAllocator, &_vkSepaphoreRenderFinished));
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
			VK_CHECK(result);
		}
		return;
	}

	// wait till finish of previos command buffer
	if (const VkResult result = vkWaitForFences(_device.get(), 1, &_vkSubmitQueueFences[imageIndex], true, UINT32_MAX); VK_TIMEOUT == result)
	{
		return;
	}
	else
	{
		VK_CHECK(result);
	}
	vkResetFences(_device.get(), 1, &_vkSubmitQueueFences[imageIndex]);

	prepareCommandBuffer(imageIndex);

	VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore waitSemaphores[]{_vkSepaphoreImageAvaible};
	VkSemaphore signalSemaphores[]{_vkSepaphoreRenderFinished};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_vkGraphicsCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(_device.getGraphicsQueue(), 1, &submitInfo, _vkSubmitQueueFences[imageIndex]));

	VkSwapchainKHR swapchains[]{_vkSwapchain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(_device.getPresentQueue(), &presentInfo);
}

void vk_renderer::cleanupSwapchain(VkSwapchainKHR& swapchain)
{
	_device.waitIdle();

	vkDestroyRenderPass(_device.get(), _vkRenderPass, _vkAllocator);

	for (auto frameBuffer : _vkFramebuffers)
	{
		vkDestroyFramebuffer(_device.get(), frameBuffer, _vkAllocator);
	}
	_vkFramebuffers.clear();

	for (auto imageView : _vkSwapchainImageViews)
	{
		vkDestroyImageView(_device.get(), imageView, _vkAllocator);
	}
	_vkSwapchainImageViews.clear();

	vkDestroySwapchainKHR(_device.get(), swapchain, _vkAllocator);
}

void vk_renderer::recreateSwapchain()
{
	_surface.setup(_physicalDevice.get());

	createSwapchain();
	createImageViews();

	createRenderPass();
	createFramebuffers();

	for (vk_mesh* mesh : _meshes)
	{
		mesh->recreatePipeline(_vkRenderPass, _surface.getCapabilities().currentExtent);
	}
}

void vk_renderer::prepareCommandBuffer(uint32_t imageIndex)
{
	VkCommandBuffer& commandBuffer = _vkGraphicsCommandBuffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	const VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _vkRenderPass;
	renderPassInfo.framebuffer = _vkFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _surface.getCapabilities().currentExtent;
	renderPassInfo.clearValueCount = 4;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	for (vk_mesh* mesh : _meshes)
	{
		mesh->bindToCmdBuffer(commandBuffer, imageIndex);
		mesh->beforeSubmitUpdate(imageIndex);
	}
	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void vk_renderer::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
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

	vkQueueSubmit(_device.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_device.getTransferQueue());
	vkFreeCommandBuffers(_device.get(), _vkTransferCommandPool, 1, &commandBuffer);
}