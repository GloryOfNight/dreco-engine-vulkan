#include "view.hxx"

#include "renderer.hxx"
#include "utils.hxx"

#include <thread>

void de::vulkan::view::init(vk::SurfaceKHR surface, uint32_t viewIndex)
{
	_viewIndex = viewIndex;
	_surface = surface;

	auto renderer = renderer::get();
	auto physicalDevice = renderer->getPhysicalDevice();
	auto device = renderer->getDevice();

	_surfaceFormat = utils::findSurfaceFormat(physicalDevice, surface);
	_presentMode = utils::findPresentMode(physicalDevice, surface);

	_settings.init();

	updateExtent(physicalDevice);

	createSwapchain(physicalDevice, device);

	createImageViews(device);

	createImageCommandBuffers(device, renderer->getTransferCommandPool());

	_depthImage.create(viewIndex);
	_msaaImage.create(viewIndex);

	createRenderPass(device);
	createFramebuffers(device);
	createFences(device);
	createSemaphores(device);
}

void de::vulkan::view::recreateSwapchain()
{
	auto renderer = renderer::get();
	auto physicalDevice = renderer->getPhysicalDevice();
	auto device = renderer->getDevice();

	if (0 == _currentExtent.height || 0 == _currentExtent.width)
	{
		return;
	}

	createSwapchain(physicalDevice, device);
	createImageViews(device);

	createRenderPass(device);

	_depthImage.recreate();
	_msaaImage.recreate();

	createFramebuffers(device);
}

void de::vulkan::view::destroy()
{
	auto renderer = renderer::get();
	auto instance = renderer->getInstance();
	auto physicalDevice = renderer->getPhysicalDevice();
	auto device = renderer->getDevice();

	[[maybe_unused]] const auto waitResult = device.waitForFences(_submitQueueFences, true, UINT32_MAX);
	for (auto& fence : _submitQueueFences)
	{
		device.destroyFence(fence);
	}

	cleanupSwapchain(device, _swapchain);

	device.destroySemaphore(_semaphoreImageAvailable);
	device.destroySemaphore(_semaphoreRenderFinished);

	_depthImage.destroy();
	_msaaImage.destroy();

	instance.destroy(_surface);

	new (this) view();
}

bool de::vulkan::view::updateExtent(vk::PhysicalDevice physicalDevice)
{
	const vk::Extent2D newExtent = physicalDevice.getSurfaceCapabilitiesKHR(_surface).currentExtent;
	if (_currentExtent != newExtent)
	{
		_currentExtent = newExtent;
		return true;
	}
	return false;
}

void de::vulkan::view::setViewMatrix(const de::math::mat4& viewMatrix)
{
	_viewMatrix = viewMatrix;
}
void de::vulkan::view::applySettings(settings&& newSettings)
{
	if (_settings != newSettings)
	{
		_settings = newSettings;

		recreateSwapchain();

		auto& mats = renderer::get()->getMaterials();
		for (auto& [name, mat] : mats)
		{
			mat->viewUpdated(_viewIndex);
		}
	}
}

uint32_t de::vulkan::view::acquireNextImageIndex()
{
	const auto renderer = renderer::get();
	auto device = renderer->getDevice();

	vk::ResultValue<uint32_t> aquireNextImageResult = vk::ResultValue<uint32_t>(vk::Result{}, UINT32_MAX);
	try
	{
		aquireNextImageResult = device.acquireNextImageKHR(_swapchain, UINT32_MAX, _semaphoreImageAvailable, nullptr);
	}
	catch (vk::OutOfDateKHRError outOfDateKHRError)
	{
		return UINT32_MAX;
	}

	const uint32_t imageIndex = aquireNextImageResult.value;

	if (vk::Result::eSuccess != aquireNextImageResult.result && vk::Result::eSuboptimalKHR != aquireNextImageResult.result)
	{
		if (vk::Result::eErrorOutOfDateKHR == aquireNextImageResult.result)
		{
			recreateSwapchain();
		}
		return UINT32_MAX;
	}
	return aquireNextImageResult.value;
}

vk::CommandBuffer de::vulkan::view::beginCommandBuffer(uint32_t imageIndex)
{
	const auto renderer = renderer::get();
	auto device = renderer->getDevice();

	const std::array<vk::Fence, 1> waitFences{_submitQueueFences[imageIndex]};
	const vk::Result waitFencesResult = device.waitForFences(waitFences, true, UINT32_MAX);
	if (waitFencesResult == vk::Result::eTimeout)
		return nullptr;

	device.resetFences(waitFences);

	vk::CommandBuffer commandBuffer = _imageCommandBuffers[imageIndex];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0F, 0U);

	const vk::RenderPassBeginInfo renderPassBeginInfo =
		vk::RenderPassBeginInfo()
			.setRenderPass(_renderPass)
			.setFramebuffer(_framebuffers[imageIndex])
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), _currentExtent))
			.setClearValues(clearValues);

	commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	return commandBuffer;
}

void de::vulkan::view::endCommandBuffer(vk::CommandBuffer commandBuffer)
{
	commandBuffer.endRenderPass();
	commandBuffer.end();
}

void de::vulkan::view::submitCommandBuffer(uint32_t imageIndex, vk::CommandBuffer commandBuffer)
{
	const auto renderer = renderer::get();
	auto graphicsQueue = renderer->getGraphicsQueue();

	const std::array<vk::Semaphore, 1> submitWaitSemaphores = {_semaphoreImageAvailable};
	const std::array<vk::Semaphore, 1> submitSignalSemaphores = {_semaphoreRenderFinished};
	const std::array<vk::PipelineStageFlags, 1> submitWaitDstStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::CommandBuffer, 1> submitCommandBuffers = {commandBuffer};

	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo()
			.setWaitSemaphores(submitWaitSemaphores)
			.setSignalSemaphores(submitSignalSemaphores)
			.setWaitDstStageMask(submitWaitDstStages)
			.setCommandBuffers(submitCommandBuffers);

	graphicsQueue.submit(submitInfo, _submitQueueFences[imageIndex]);

	const vk::PresentInfoKHR presentInfo =
		vk::PresentInfoKHR()
			.setWaitSemaphores(submitSignalSemaphores)
			.setSwapchains({1, &_swapchain})
			.setImageIndices({1, &imageIndex});

	try
	{
		const vk::Result presentResult = graphicsQueue.presentKHR(presentInfo);

		if (vk::Result::eSuboptimalKHR == presentResult)
		{
			recreateSwapchain();
		}
	}
	catch (vk::OutOfDateKHRError error)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		DE_LOG(Error, "OutOfDateKHRError");
	}
}

vk::SharingMode de::vulkan::view::getSharingMode() const
{
	return vk::SharingMode::eExclusive;
}

uint32_t de::vulkan::view::getImageCount() const
{
	return _swapchainImageViews.size();
}

void de::vulkan::view::createSwapchain(vk::PhysicalDevice physicalDevice, vk::Device device)
{
	const auto sharingMode{getSharingMode()};
	const auto queueFamilyIndexes{renderer::get()->getQueueFamilyIndices()};

	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
	const uint32_t minImageCount = surfaceCapabilities.maxImageCount >= 3 ? 3 : surfaceCapabilities.minImageCount;

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo =
		vk::SwapchainCreateInfoKHR()
			.setSurface(_surface)
			.setMinImageCount(minImageCount)
			.setImageFormat(_surfaceFormat.format)
			.setImageColorSpace(_surfaceFormat.colorSpace)
			.setImageExtent(_currentExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(static_cast<vk::SharingMode>(sharingMode))
			.setQueueFamilyIndices(queueFamilyIndexes)
			.setPreTransform(surfaceCapabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(_presentMode)
			.setClipped(VK_TRUE)
			.setOldSwapchain(_swapchain);

	_swapchain = device.createSwapchainKHR(swapchainCreateInfo);
	if (swapchainCreateInfo.oldSwapchain)
	{
		cleanupSwapchain(device, swapchainCreateInfo.oldSwapchain);
	}
}

void de::vulkan::view::cleanupSwapchain(vk::Device device, vk::SwapchainKHR swapchain)
{
	device.waitIdle();

	device.destroyRenderPass(_renderPass);

	for (auto frameBuffer : _framebuffers)
	{
		device.destroyFramebuffer(frameBuffer);
	}
	_framebuffers.clear();

	for (auto imageView : _swapchainImageViews)
	{
		device.destroyImageView(imageView);
	}
	_swapchainImageViews.clear();

	device.destroySwapchainKHR(swapchain);
}

void de::vulkan::view::createImageViews(vk::Device device)
{
	const auto swapchainImages = device.getSwapchainImagesKHR(_swapchain);
	const size_t imageCount = swapchainImages.size();

	_swapchainImageViews.resize(imageCount);
	for (size_t i = 0; i < imageCount; ++i)
	{
		const vk::ComponentMapping imageViewComponents =
			vk::ComponentMapping()
				.setR(vk::ComponentSwizzle::eIdentity)
				.setG(vk::ComponentSwizzle::eIdentity)
				.setB(vk::ComponentSwizzle::eIdentity)
				.setA(vk::ComponentSwizzle::eIdentity);

		const vk::ImageSubresourceRange imageSubresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

		const vk::ImageViewCreateInfo imageViewCreateInfo =
			vk::ImageViewCreateInfo()
				.setImage(swapchainImages[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(_surfaceFormat.format)
				.setComponents(imageViewComponents)
				.setSubresourceRange(imageSubresourceRange);

		_swapchainImageViews[i] = device.createImageView(imageViewCreateInfo);
	}
}

void de::vulkan::view::createRenderPass(vk::Device device)
{
	const auto sampleCount = _settings.getSampleCount();
	const bool isMultisamplingSupported = _settings.IsMultisamplingSupported();

	std::vector<vk::AttachmentDescription> attachmentsDescriptions;

	std::vector<vk::AttachmentReference> attachmentReferences;
	std::vector<vk::AttachmentReference> resolveAttachmentReferences;

	attachmentsDescriptions.emplace_back() // color
		.setFormat(_surfaceFormat.format)
		.setSamples(sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(isMultisamplingSupported ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(isMultisamplingSupported ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);
	attachmentReferences.push_back(vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal));

	attachmentsDescriptions.emplace_back() // depth
		.setFormat(_depthImage.getFormat())
		.setSamples(sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachmentReferences.push_back(vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal));

	if (isMultisamplingSupported)
	{
		attachmentsDescriptions.emplace_back() // color msaa
			.setFormat(_surfaceFormat.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		resolveAttachmentReferences.push_back(vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal));
	}

	const vk::SubpassDescription subpassDescription =
		vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments({1, &attachmentReferences[0]})
			.setPDepthStencilAttachment(&attachmentReferences[1])
			.setPResolveAttachments(resolveAttachmentReferences.data());

	const vk::SubpassDependency subpassDependecy =
		vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits(0))
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	const vk::RenderPassCreateInfo renderPassCreateInfo =
		vk::RenderPassCreateInfo()
			.setAttachments(attachmentsDescriptions)
			.setSubpasses({1, &subpassDescription})
			.setDependencies({1, &subpassDependecy});

	_renderPass = device.createRenderPass(renderPassCreateInfo);
}

void de::vulkan::view::createFramebuffers(vk::Device device)
{
	const size_t imageCount = getImageCount();
	_framebuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; ++i)
	{
		std::vector<vk::ImageView> attachments;
		attachments.reserve(3);

		if (_settings.getSampleCount() != vk::SampleCountFlagBits::e1)
		{
			attachments.push_back(_msaaImage.getImageView());
			attachments.push_back(_depthImage.getImageView());
			attachments.push_back(_swapchainImageViews[i]);
		}
		else
		{
			attachments.push_back(_swapchainImageViews[i]);
			attachments.push_back(_depthImage.getImageView());
		}

		const vk::FramebufferCreateInfo framebufferCreateInfo =
			vk::FramebufferCreateInfo()
				.setRenderPass(_renderPass)
				.setAttachments(attachments)
				.setWidth(_currentExtent.width)
				.setHeight(_currentExtent.height)
				.setLayers(1);

		_framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
	}
}

void de::vulkan::view::createImageCommandBuffers(vk::Device device, vk::CommandPool transferCommandPool)
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(getImageCount())
			.setCommandPool(transferCommandPool);
	_imageCommandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void de::vulkan::view::createFences(vk::Device device)
{
	_submitQueueFences.resize(getImageCount());
	for (auto& fence : _submitQueueFences)
	{
		const vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		fence = device.createFence(fenceCreateInfo);
	}
}

void de::vulkan::view::createSemaphores(vk::Device device)
{
	_semaphoreImageAvailable = device.createSemaphore(vk::SemaphoreCreateInfo());
	_semaphoreRenderFinished = device.createSemaphore(vk::SemaphoreCreateInfo());
}
