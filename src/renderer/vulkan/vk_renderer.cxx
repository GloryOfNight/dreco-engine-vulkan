#include "vk_renderer.hxx"
#include "vk_utils.hxx"
#include "vk_queue_family.hxx"
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

vk_renderer::vk_renderer(engine* eng)
	: _engine(eng)
	, mAllocator(nullptr)
	, surface(&mInstance)
	, physicalDevice(&mInstance)
	, device()
{
	createWindow();
	createInstance();
	surface.create(getWindow());
	physicalDevice.setup(surface.get());
	surface.setup(physicalDevice.get());
	queueFamily.setup(physicalDevice.get(), surface.get());
	device.create(physicalDevice, queueFamily);

	createSemaphores();
	createCommandPool();

	recreateSwapchain();
}

vk_renderer::~vk_renderer()
{
	device.waitIdle();

	cleanupSwapchain(mSwapchain);

	vkMesh.destroy();

	vkDestroySemaphore(device.get(), mSepaphore_Image_Avaible, mAllocator);
	vkDestroySemaphore(device.get(), mSepaphore_Render_Finished, mAllocator);
	vkDestroyCommandPool(device.get(), mGraphicsCommandPool, mAllocator);
	vkDestroyCommandPool(device.get(), mTransferCommandPool, mAllocator);

	device.destroy();
	SDL_DestroyWindow(window);
	surface.destroy();
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
	window = SDL_CreateWindow("dreco-test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720,
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

	VK_CHECK(vkCreateInstance(&instance_info, mAllocator, &mInstance));
}

void vk_renderer::createSwapchain()
{
	VkSharingMode sharingMode{queueFamily.getSharingMode()};
	std::vector<uint32_t> queueFamilyIndexes
	{
		queueFamily.getGraphicsIndex(),
		queueFamily.getTransferIndex(),
		queueFamily.getPresentIndex()
	};

	const VkSurfaceFormatKHR& surfaceFormat{surface.getFormat()};
	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{surface.getCapabilities()};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface.get();
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
	swapchainCreateInfo.presentMode = surface.getPresentMode();
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = mSwapchain;

	VK_CHECK(vkCreateSwapchainKHR(device.get(), &swapchainCreateInfo, mAllocator, &mSwapchain));

	if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void vk_renderer::createImageViews()
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device.get(), mSwapchain, &imageCount, nullptr);

	std::vector<VkImage> mSwapchainImages;
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device.get(), mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = surface.getFormat().format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		VK_CHECK(vkCreateImageView(device.get(), &imageViewCreateInfo, mAllocator, &mSwapchainImageViews[i]));
	}
}

void vk_renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = surface.getFormat().format;
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

	VK_CHECK(vkCreateRenderPass(device.get(), &renderPassCreateInfo, mAllocator, &mRenderPass));
}

void vk_renderer::createFramebuffers()
{
	mFramebuffers.resize(mSwapchainImageViews.size());

	const VkSurfaceCapabilitiesKHR& surfaceCapabilities{surface.getCapabilities()};

	for (uint32_t i = 0; i < mSwapchainImageViews.size(); ++i)
	{
		VkImageView attachments[]{mSwapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = mRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height = surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(device.get(), &framebufferCreateInfo, mAllocator, &mFramebuffers[i]));
	}
}

void vk_renderer::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamily.getGraphicsIndex();
	commandPoolCreateInfo.flags = 0;

	VK_CHECK(vkCreateCommandPool(device.get(), &commandPoolCreateInfo, mAllocator, &mGraphicsCommandPool));

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamily.getTransferIndex();
	commandPoolCreateInfo.flags = 0;

	VK_CHECK(vkCreateCommandPool(device.get(), &commandPoolCreateInfo, mAllocator, &mTransferCommandPool));
}

void vk_renderer::createCommandBuffers()
{
	mGraphicsCommandBuffers.resize(mFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mGraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(mGraphicsCommandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(device.get(), &allocInfo, mGraphicsCommandBuffers.data()));
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

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = surface.getCapabilities().currentExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkMesh.bindToCmdBuffer(commandBuffer, i);
		vkCmdEndRenderPass(commandBuffer);
		
		VK_CHECK(vkEndCommandBuffer(commandBuffer));
	}
}

void vk_renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK(vkCreateSemaphore(device.get(), &semaphoreInfo, mAllocator, &mSepaphore_Image_Avaible));
	VK_CHECK(vkCreateSemaphore(device.get(), &semaphoreInfo, mAllocator, &mSepaphore_Render_Finished));
}

void vk_renderer::drawFrame()
{
	uint32_t imageIndex;
	if (VkResult result = vkAcquireNextImageKHR(device.get(), mSwapchain, UINT32_MAX,mSepaphore_Image_Avaible, VK_NULL_HANDLE, &imageIndex);
		VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			recreateSwapchain();
		}
		else 
		{
			VK_CHECK(result);
		}
		return;
	}

	VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore waitSemaphores[]{mSepaphore_Image_Avaible};
	VkSemaphore signalSemaphores[]{mSepaphore_Render_Finished};

	vkMesh.beforeSubmitUpdate(imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mGraphicsCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	VkSwapchainKHR swapchains[]{mSwapchain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
}

void vk_renderer::cleanupSwapchain(VkSwapchainKHR& swapchain)
{
	device.waitIdle();

	vkFreeCommandBuffers(device.get(), mGraphicsCommandPool, static_cast<uint32_t>(mGraphicsCommandBuffers.size()),
		mGraphicsCommandBuffers.data());
	mGraphicsCommandBuffers.clear();

	vkDestroyRenderPass(device.get(), mRenderPass, mAllocator);

	for (auto frameBuffer : mFramebuffers)
	{
		vkDestroyFramebuffer(device.get(), frameBuffer, mAllocator);
	}
	mFramebuffers.clear();

	for (auto imageView : mSwapchainImageViews)
	{
		vkDestroyImageView(device.get(), imageView, mAllocator);
	}
	mSwapchainImageViews.clear();

	vkDestroySwapchainKHR(device.get(), swapchain, mAllocator);
}

void vk_renderer::recreateSwapchain()
{
	surface.setup(physicalDevice.get());

	createSwapchain();
	createImageViews();
	
	createRenderPass();
	createFramebuffers();
	createCommandBuffers();

	vk_mesh_create_info mesh_create_info
	{
		&device,
		&queueFamily,
		&physicalDevice,
		mRenderPass,
		surface.getCapabilities().currentExtent,
		static_cast<uint32_t>(mSwapchainImageViews.size())
	};
	vkMesh.destroy();
	vkMesh.create(mesh_create_info);

	recordCommandBuffers();
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
	vkAllocateCommandBuffers(device.get(), &commandBufferAllocateInfo, &commandBuffer);

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

	vkQueueSubmit(device.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getTransferQueue());
	vkFreeCommandBuffers(device.get(), mTransferCommandPool, 1, &commandBuffer);
}