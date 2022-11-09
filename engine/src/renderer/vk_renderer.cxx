#include "vk_renderer.hxx"

#include "core/engine.hxx"
#include "core/threads/thread_pool.hxx"
#include "core/utils/file_utils.hxx"
#include "game_objects/camera.hxx"

#include "dreco.hxx"
#include "vk_exceptions.hxx"
#include "vk_mesh.hxx"
#include "vk_utils.hxx"

#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <chrono>

vk_renderer::~vk_renderer()
{
	exit();
}

vk_renderer* vk_renderer::get()
{
	if (auto eng = engine::get())
	{
		return &eng->getRenderer();
	}
	return nullptr;
}

bool vk_renderer::isSupported()
{
	return NULL != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void vk_renderer::init()
{
	_apiVersion = vk::enumerateInstanceVersion();

	createWindow();
	createInstance();
	createSurface();
	createPhysicalDevice();

	// if instance was created with api version bigger that device can support, recreate stuff with lower version
	if (const uint32_t deviceApiVersion = _physicalDevice.getProperties().apiVersion; _apiVersion > deviceApiVersion)
	{
		_apiVersion = deviceApiVersion;
		_instance.destroySurfaceKHR(_surface);
		_instance.destroy();

		createInstance();
		createSurface();
		createPhysicalDevice();
	}
	updateExtent();
	_settings.init(this);

	createDevice();

	createQueues();

	createSwapchain();
	createImageViews();

	createCommandPools();
	createCommandBuffers();

	_depthImage.create();
	_msaaImage.create();

	createRenderPass();

	createFramebuffers();

	createFences();
	createSemaphores();

	createBufferPools();
	createCameraBuffer();

	_placeholderTextureImage.create(image_data::makePlaceholder());
}

void vk_renderer::exit()
{
	if (!_device)
	{
		return;
	}

	_device.waitIdle();

	_scenes.clear();
	_shaders.clear();

	for (auto& fence : _submitQueueFences)
	{
		_device.destroyFence(fence);
	}

	cleanupSwapchain(_swapchain);

	_placeholderTextureImage.destroy();

	_device.destroySemaphore(_semaphoreImageAvaible);
	_device.destroySemaphore(_semaphoreRenderFinished);

	_device.destroyCommandPool(_graphicsCommandPool);
	_device.destroyCommandPool(_transferCommandPool);

	_depthImage.destroy();
	_msaaImage.destroy();
	_bpVertIndx.destroy();
	_bpUniforms.destroy();
	_bpTransfer.destroy();
	_device.destroy();

	_instance.destroy(_surface);
	_instance.destroy();

	SDL_DestroyWindow(_window);

	new (this) vk_renderer();
}

void vk_renderer::tick(double deltaTime)
{
	updateCameraBuffer();
	if (updateExtent())
	{
		recreateSwapchain();
	}
	drawFrame();
}

void vk_renderer::loadModel(const gltf::model& scn)
{
	_scenes.emplace_back(new vk_scene())->create(scn);
}

vk_shader::shared vk_renderer::loadShader(const std::string_view& path)
{
	const auto shader = _shaders.try_emplace(path.data());
	if (shader.second)
	{
		shader.first->second = vk_shader::shared(new vk_shader());
		shader.first->second->create(path);
	}
	return shader.first->second;
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
	return _swapchainImageViews.size();
}

vk::SharingMode vk_renderer::getSharingMode() const
{
	return vk::SharingMode::eExclusive;
}

std::vector<uint32_t> vk_renderer::getQueueFamilyIndices() const
{
	if (_graphicsQueueIndex == _transferQueueIndex)
	{
		return std::vector<uint32_t>{_graphicsQueueIndex};
	}
	return std::vector<uint32_t>{_graphicsQueueIndex, _transferQueueIndex};
}

vk::CommandBuffer vk_renderer::beginSingleTimeTransferCommands()
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
			.setCommandPool(_transferCommandPool);

	vk::CommandBuffer commandBuffer = _device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo =
		vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	return commandBuffer;
}

void vk_renderer::submitSingleTimeTransferCommands(vk::CommandBuffer commandBuffer)
{
	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo().setCommandBuffers({1, &commandBuffer});

	submitSingleTimeTransferCommands({submitInfo});
}

void vk_renderer::submitSingleTimeTransferCommands(const std::vector<vk::SubmitInfo>& submits)
{
	_transferQueue.submit(submits, nullptr);
	_transferQueue.waitIdle();
}

void vk_renderer::applySettings()
{
	recreateSwapchain();
}

void vk_renderer::createWindow()
{
	_window = SDL_CreateWindow("dreco-launcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	_windowId = SDL_GetWindowID(_window);
}

void vk_renderer::createInstance()
{
	std::vector<const char*> instanceExtensions;

	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(_window, &count, nullptr);
	instanceExtensions.resize(count);
	SDL_Vulkan_GetInstanceExtensions(_window, &count, instanceExtensions.data() + 0);

	const auto allInstanceLayers = vk::enumerateInstanceLayerProperties();

	std::vector<const char*> instanceLayers{};
	for (const auto& layerProperty : allInstanceLayers)
	{
		const auto push_layer_if_available_lam = [&instanceLayers, &layerProperty](const std::string_view layer) -> void
		{
			if (layerProperty.layerName == layer)
				instanceLayers.push_back(layer.data());
		};

#ifdef DRECO_VK_USE_VALIDATION
		push_layer_if_available_lam("VK_LAYER_KHRONOS_validation");
#endif

#ifdef DRECO_VK_USE_MESA_OVERLAY
		push_layer_if_available_lam("VK_LAYER_MESA_overlay");
#endif

#ifdef DRECO_VK_USE_LUNAR_MONITOR
		push_layer_if_available_lam("VK_LAYER_LUNARG_monitor");
#endif
	}

	const vk::ApplicationInfo applicationInfo("dreco-launcher", 0, "dreco", 0, _apiVersion);
	const vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, instanceLayers, instanceExtensions);
	_instance = vk::createInstance(instanceCreateInfo);
}

void vk_renderer::createSurface()
{
	VkSurfaceKHR newSurface;
	if (SDL_Vulkan_CreateSurface(_window, _instance, &newSurface) == SDL_TRUE)
	{
		_surface = newSurface;
	}
}

void vk_renderer::createPhysicalDevice()
{
	const auto physicalDevices = _instance.enumeratePhysicalDevices();

	// clang-format off
	auto isGpuSuitSurface = [this](const vk::PhysicalDevice physicalDevice) -> bool 
	{
		const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
		const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
		for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
		{
			if (physicalDevice.getSurfaceSupportKHR(i, _surface))
			{
				return true;
			}
		}
		return false;
	};
	// clang-format on

	for (vk::PhysicalDevice physicalDevice : physicalDevices)
	{
		if (isGpuSuitSurface(physicalDevice))
		{
			const auto physicalDeviceProperties = physicalDevice.getProperties();
			if (vk::PhysicalDeviceType::eCpu == physicalDeviceProperties.deviceType ||
				vk::PhysicalDeviceType::eOther == physicalDeviceProperties.deviceType)
			{
				continue;
			}

			_physicalDevice = physicalDevice;
			if (vk::PhysicalDeviceType::eDiscreteGpu == physicalDeviceProperties.deviceType)
			{
				break;
			}
		}
	}

	if (!_physicalDevice)
	{
		throw vk_except::no_gpu();
	}
}

void vk_renderer::createDevice()
{
	const auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
	queueCreateInfoList.reserve(queueFamilyProperties.size());

	for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		constexpr float priority = 1.F;
		queueCreateInfoList.emplace_back()
			.setQueueFamilyIndex(i)
			.setQueueCount(1)
			.setQueuePriorities(priority);
	}

	const std::vector<const char*> enabledExtensions{"VK_KHR_swapchain"};
	const std::vector<const char*> enabledLayers{
#ifdef DRECO_VK_USE_MESA_OVERLAY
		"VK_LAYER_MESA_overlay",
#endif
#ifdef DRECO_VK_USE_LUNAR_MONITOR
		"VK_LAYER_LUNARG_monitor",
#endif
	};

	const vk::PhysicalDeviceFeatures physicalDeviceFeatures = _physicalDevice.getFeatures();

	const vk::DeviceCreateInfo deviceCreateInfo =
		vk::DeviceCreateInfo()
			.setQueueCreateInfos(queueCreateInfoList)
			.setPEnabledLayerNames(enabledLayers)
			.setPEnabledExtensionNames(enabledExtensions)
			.setPEnabledFeatures(&physicalDeviceFeatures);

	_device = _physicalDevice.createDevice(deviceCreateInfo);
}

void vk_renderer::createQueues()
{
	const auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
	const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
	for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
	{
		const vk::Bool32 isSupported = _physicalDevice.getSurfaceSupportKHR(i, _surface);
		if (isSupported)
		{
			const auto queueFlags = queueFamilyProperties[i].queueFlags;
			if ((queueFlags & vk::QueueFlagBits::eGraphics) && (queueFlags & vk::QueueFlagBits::eTransfer))
			{
				_graphicsQueueIndex = i;
				_transferQueueIndex = i;
				break;
			}
		}
	}
	_graphicsQueue = _device.getQueue(_graphicsQueueIndex, 0);
	_transferQueue = _device.getQueue(_transferQueueIndex, 0);
}

void vk_renderer::createSwapchain()
{
	const auto sharingMode{getSharingMode()};
	const auto queueFamilyIndexes{getQueueFamilyIndices()};

	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
	const vk::PresentModeKHR presentMode = _settings.getPresentMode();
	const vk::SurfaceFormatKHR surfaceFormat = _settings.getSurfaceFormat();
	const uint32_t minImageCount = surfaceCapabilities.maxImageCount >= 3 ? 3 : surfaceCapabilities.minImageCount;

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo =
		vk::SwapchainCreateInfoKHR()
			.setSurface(_surface)
			.setMinImageCount(minImageCount)
			.setImageFormat(surfaceFormat.format)
			.setImageColorSpace(surfaceFormat.colorSpace)
			.setImageExtent(_currentExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(static_cast<vk::SharingMode>(sharingMode))
			.setQueueFamilyIndices(queueFamilyIndexes)
			.setPreTransform(surfaceCapabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(presentMode)
			.setClipped(VK_TRUE)
			.setOldSwapchain(_swapchain);

	_swapchain = _device.createSwapchainKHR(swapchainCreateInfo);
	if (swapchainCreateInfo.oldSwapchain)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void vk_renderer::createImageViews()
{
	const auto swapchainImages = _device.getSwapchainImagesKHR(_swapchain);
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
				.setFormat(_settings.getSurfaceFormat().format)
				.setComponents(imageViewComponents)
				.setSubresourceRange(imageSubresourceRange);

		_swapchainImageViews[i] = _device.createImageView(imageViewCreateInfo);
	}
}

void vk_renderer::createRenderPass()
{
	const vk::SampleCountFlagBits sampleCount = _settings.getPrefferedSampleCount();
	const bool isSamplingSupported = _settings.getIsSamplingSupported();

	std::vector<vk::AttachmentDescription> attachmentsDescriptions;

	std::vector<vk::AttachmentReference> attachmentReferences;
	std::vector<vk::AttachmentReference> resolveAttachmentReferences;

	attachmentsDescriptions.emplace_back() // color
		.setFormat(_settings.getSurfaceFormat().format)
		.setSamples(sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(isSamplingSupported ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(isSamplingSupported ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);
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

	if (isSamplingSupported)
	{
		attachmentsDescriptions.emplace_back() // color msaa
			.setFormat(_settings.getSurfaceFormat().format)
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

	_renderPass = _device.createRenderPass(renderPassCreateInfo);
}

void vk_renderer::createFramebuffers()
{
	const size_t imageCount = getImageCount();
	_framebuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; ++i)
	{
		std::vector<vk::ImageView> attachments;
		attachments.reserve(3);

		if (_settings.getIsSamplingSupported())
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

		_framebuffers[i] = _device.createFramebuffer(framebufferCreateInfo);
	}
}

void vk_renderer::createCommandPools()
{
	const vk::CommandPoolCreateInfo graphicsCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
															 .setQueueFamilyIndex(_graphicsQueueIndex);
	_graphicsCommandPool = _device.createCommandPool(graphicsCreateInfo);

	const vk::CommandPoolCreateInfo transferCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
															 .setQueueFamilyIndex(_transferQueueIndex);
	_transferCommandPool = _device.createCommandPool(transferCreateInfo);
}

void vk_renderer::createCommandBuffers()
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(getImageCount())
			.setCommandPool(_transferCommandPool);
	_imageCommandBuffers = _device.allocateCommandBuffers(commandBufferAllocateInfo);
}

inline void vk_renderer::createFences()
{
	_submitQueueFences.resize(getImageCount());
	for (auto& fence : _submitQueueFences)
	{
		const vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		fence = _device.createFence(fenceCreateInfo);
	}
}

void vk_renderer::createSemaphores()
{
	_semaphoreImageAvaible = _device.createSemaphore(vk::SemaphoreCreateInfo());
	_semaphoreRenderFinished = _device.createSemaphore(vk::SemaphoreCreateInfo());
}

void vk_renderer::createBufferPools()
{
	constexpr auto vertIndxUsage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	constexpr auto vertIndxSize = 256 * 1024 * 1024;
	_bpVertIndx.allocate(vk_utils::memory_property::device, vertIndxUsage, vertIndxSize);

	constexpr auto uniformsUsage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
	constexpr auto uniformsSize = 64 * 1024 * 1024;
	_bpUniforms.allocate(vk_utils::memory_property::device, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, uniformsSize);

	constexpr auto transferUsage = vk::BufferUsageFlagBits::eTransferSrc;
	constexpr auto transferSize = 256 * 1024 * 1024;
	_bpTransfer.allocate(vk_utils::memory_property::host, transferUsage, transferSize);
}

void vk_renderer::createCameraBuffer()
{
	_cameraDataBufferId = getUniformBufferPool().makeBuffer(sizeof(camera_data));
}

void vk_renderer::drawFrame()
{
	vk::ResultValue<uint32_t> aquireNextImageResult = vk::ResultValue<uint32_t>(vk::Result{}, UINT32_MAX);
	try
	{
		aquireNextImageResult = _device.acquireNextImageKHR(_swapchain, UINT32_MAX, _semaphoreImageAvaible, nullptr);
	}
	catch (vk::OutOfDateKHRError outOfDateKHRError)
	{
		return;
	}

	const uint32_t imageIndex = aquireNextImageResult.value;

	if (vk::Result::eSuccess != aquireNextImageResult.result && vk::Result::eSuboptimalKHR != aquireNextImageResult.result)
	{
		if (vk::Result::eErrorOutOfDateKHR == aquireNextImageResult.result)
		{
			recreateSwapchain();
		}
		return;
	}

	const std::array<vk::Fence, 1> waitFences{_submitQueueFences[imageIndex]};
	const vk::Result waitFencesResult = _device.waitForFences(waitFences, true, UINT32_MAX);
	if (waitFencesResult == vk::Result::eTimeout)
		return;

	_device.resetFences(waitFences);

	vk::CommandBuffer commandBuffer = prepareCommandBuffer(imageIndex);

	const std::array<vk::Semaphore, 1> submitWaitSemaphores = {_semaphoreImageAvaible};
	const std::array<vk::Semaphore, 1> submitSignalSemaphores = {_semaphoreRenderFinished};
	const std::array<vk::PipelineStageFlags, 1> submitWaitDstStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::CommandBuffer, 1> submitCommandBuffers = {commandBuffer};

	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo()
			.setWaitSemaphores(submitWaitSemaphores)
			.setSignalSemaphores(submitSignalSemaphores)
			.setWaitDstStageMask(submitWaitDstStages)
			.setCommandBuffers(submitCommandBuffers);

	_graphicsQueue.submit(submitInfo, _submitQueueFences[imageIndex]);

	const vk::PresentInfoKHR presentInfo =
		vk::PresentInfoKHR()
			.setWaitSemaphores(submitSignalSemaphores)
			.setSwapchains({1, &_swapchain})
			.setImageIndices({1, &imageIndex});

	try
	{
		const vk::Result presentResult = _graphicsQueue.presentKHR(presentInfo);

		if (vk::Result::eSuboptimalKHR == aquireNextImageResult.result ||
			vk::Result::eSuboptimalKHR == presentResult)
		{
			recreateSwapchain();
		}
	}
	catch (vk::OutOfDateKHRError error)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		SDL_UpdateWindowSurface(_window);

		DE_LOG(Error, "OutOfDateKHRError");
	}
}

void vk_renderer::setCameraData(const mat4& inView, const mat4 inProj)
{
	_cameraData.view = inView;
	_cameraData.viewProj = inView * inProj;
}

void vk_renderer::updateCameraBuffer()
{
	constexpr auto size = sizeof(_cameraData);

	static const auto id = _bpTransfer.makeBuffer(size);

	auto region = _bpTransfer.map(id);
	std::memcpy(region, &_cameraData, size);
	_bpTransfer.unmap(id);

	const auto copyRegion = vk::BufferCopy(0, 0, size);
	vk_buffer::copyBuffer(_bpTransfer.getBuffer(id).get(), _bpUniforms.getBuffer(_cameraDataBufferId).get(), {copyRegion});
}

bool vk_renderer::updateExtent()
{
	const vk::Extent2D newExtent = _physicalDevice.getSurfaceCapabilitiesKHR(_surface).currentExtent;
	if (_currentExtent != newExtent)
	{
		_currentExtent = newExtent;
		return true;
	}
	return false;
}

void vk_renderer::cleanupSwapchain(vk::SwapchainKHR swapchain)
{
	_device.waitIdle();

	_device.destroyRenderPass(_renderPass);

	for (auto frameBuffer : _framebuffers)
	{
		_device.destroyFramebuffer(frameBuffer);
	}
	_framebuffers.clear();

	for (auto imageView : _swapchainImageViews)
	{
		_device.destroyImageView(imageView);
	}
	_swapchainImageViews.clear();

	_device.destroySwapchainKHR(swapchain);
}

void vk_renderer::recreateSwapchain()
{
	if (0 == _currentExtent.height || 0 == _currentExtent.width)
	{
		return;
	}

	createSwapchain();
	createImageViews();

	createRenderPass();

	_depthImage.recreate();
	_msaaImage.recreate();

	createFramebuffers();

	for (auto& scene : _scenes)
	{
		scene->recreatePipelines();
	}
}

vk::CommandBuffer vk_renderer::prepareCommandBuffer(uint32_t imageIndex)
{
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
	for (auto& scene : _scenes)
	{
		scene->bindToCmdBuffer(commandBuffer);
	}
	commandBuffer.endRenderPass();

	commandBuffer.end();

	return commandBuffer;
}