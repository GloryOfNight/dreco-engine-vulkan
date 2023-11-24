#include "renderer.hxx"

#include "core/engine.hxx"
#include "game_framework/camera.hxx"
#include "math/casts.hxx"

#include "constants.hxx"
#include "dreco.hxx"
#include "utils.hxx"

#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <chrono>

de::vulkan::renderer::~renderer()
{
	exit();
}

de::vulkan::renderer* de::vulkan::renderer::get()
{
	if (auto eng = de::engine::get())
	{
		return &eng->getRenderer();
	}
	return nullptr;
}

bool de::vulkan::renderer::isSupported()
{
	return NULL != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void de::vulkan::renderer::init()
{
	_apiVersion = vk::enumerateInstanceVersion();

	{ // the base of vulkan renderer initialization

		if (SDL_Vulkan_LoadLibrary(NULL) != 0)
		{
			DE_LOG(Critical, "Failed to load vulkan library");
			std::exit(EXIT_FAILURE);
		}

		createInstance();
		createPhysicalDevice();

		// if instance was created with api version bigger that physical device can support, recreate stuff with lower version
		if (const uint32_t deviceApiVersion = _physicalDevice.getProperties().apiVersion; _apiVersion > deviceApiVersion)
		{
			DE_LOG(Info, "Api version and device api version doesn't match. Api: %i, device: %i", _apiVersion, deviceApiVersion);

			_apiVersion = deviceApiVersion;
			_instance.destroy();

			createInstance();
			createPhysicalDevice();
		}

		createDevice();

		createQueues();
		createBufferPools();
		createCommandPools();
	}

	{ // common renderer resources
		createCameraBuffer();
		_placeholderTextureImage.create(de::gltf::image::makePlaceholder(256, 256));

		{
			const auto vert = loadShader(DRECO_SHADER(constants::shaders::basicVert));
			const auto frag = loadShader(DRECO_SHADER(constants::shaders::basicFrag));
			const auto& [iter, isEmplaced] = _materials.try_emplace(constants::materials::basic, material::makeNew(vert, frag));
			iter->second->init(128);
		}
		{
			const auto vert = loadShader(DRECO_SHADER(constants::shaders::skyboxVert));
			const auto frag = loadShader(DRECO_SHADER(constants::shaders::skyboxFrag));
			const auto& [iter, isEmplaced] = _materials.try_emplace(constants::materials::skybox, material::makeNew(vert, frag));
			iter->second->setDynamicStates({vk::DynamicState::eDepthTestEnable});
			iter->second->init(128);
		}
		_skybox.init();
	}
}

void de::vulkan::renderer::exit()
{
	if (!_device)
	{
		return;
	}

	_device.waitIdle();

	_skybox.destroy();
	_placeholderTextureImage.destroy();

	_scenes.clear();
	_shaders.clear();
	_materials.clear();
	_views = {};

	_device.destroyCommandPool(_graphicsCommandPool);
	_device.destroyCommandPool(_transferCommandPool);

	_bpVertIndx.destroy();
	_bpUniforms.destroy();
	_bpTransfer.destroy();
	_device.destroy();

	_instance.destroy();

	new (this) renderer();
}

void de::vulkan::renderer::tick(double deltaTime)
{
	for (size_t i = 0; i < _views.size(); ++i)
	{
		auto& currentView = _views[i];
		if (currentView == nullptr || !currentView->isInitialized())
			continue;

		_currentDrawViewIndex = i;

		if (currentView->updateExtent(_physicalDevice))
		{
			currentView->recreateSwapchain();

			// update all materials after swapchain recreation
			for (auto& mat : _materials)
			{
				mat.second->viewUpdated(_currentDrawViewIndex);
			}

			return; // skip view draw if swapchain was recreated
		}

		const uint32_t nextImage = currentView->acquireNextImageIndex();
		if (nextImage == UINT32_MAX)
			return;

		const auto viewExtent = currentView->getCurrentExtent();
		_cameraData.view = currentView->getViewMatrix();
		_cameraData.proj = de::math::mat4::makeProjection(0.1f, 1000.f, static_cast<float>(viewExtent.width) / static_cast<float>(viewExtent.height), de::math::deg_to_rad(75.F));
		updateCameraBuffer();

		auto commandBuffer = currentView->beginCommandBuffer(nextImage);

		_skybox.drawCmd(commandBuffer);

		for (auto& scene : _scenes)
		{
			scene->bindToCmdBuffer(commandBuffer);
		}
		currentView->endCommandBuffer(commandBuffer);

		currentView->submitCommandBuffer(nextImage, commandBuffer);
	}
}

uint32_t de::vulkan::renderer::addView(SDL_Window* window)
{
	size_t viewIndex = UINT32_MAX;
	for (size_t i = 0; _views.size(); ++i)
	{
		if (_views[i] == nullptr || !_views[i]->isInitialized())
		{
			viewIndex = i;
			break;
		}
	}

	if (viewIndex == UINT32_MAX)
		return viewIndex;

	VkSurfaceKHR newSurface;
	if (SDL_Vulkan_CreateSurface(window, _instance, nullptr, &newSurface) == SDL_TRUE)
	{
		_views[viewIndex] = view::unique(new view());
		_views[viewIndex]->init(newSurface, viewIndex);
	}

	// update all materials with new view
	for (auto& mat : _materials)
	{
		mat.second->viewAdded(viewIndex);
	}

	return viewIndex;
}

void de::vulkan::renderer::removeView(uint32_t viewIndex)
{
	_views.at(viewIndex) = nullptr;

	// update all materials with new view
	for (auto& mat : _materials)
	{
		mat.second->viewRemoved(viewIndex);
	}
}

de::vulkan::material* de::vulkan::renderer::getMaterial(const std::string_view& name) const
{
	const auto material = _materials.find(name.data());
	if (material != _materials.end())
	{
		return material->second.get();
	}
	return nullptr;
}

void de::vulkan::renderer::loadModel(const de::gltf::model& scn)
{
	_scenes.emplace_back(new scene())->create(scn);
}

de::vulkan::shader::shared de::vulkan::renderer::loadShader(const std::string_view& path)
{
	const auto shader = _shaders.try_emplace(path.data());
	if (shader.second)
	{
		shader.first->second = shader::shared(new de::vulkan::shader());
		shader.first->second->create(path);
	}
	return shader.first->second;
}

uint32_t de::vulkan::renderer::getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch)
{
	major = VK_VERSION_MAJOR(_apiVersion);
	minor = VK_VERSION_MINOR(_apiVersion);
	if (nullptr != patch)
	{
		*patch = VK_VERSION_PATCH(_apiVersion);
	}
	return _apiVersion;
}

vk::SharingMode de::vulkan::renderer::getSharingMode() const
{
	return vk::SharingMode::eExclusive;
}

std::vector<uint32_t> de::vulkan::renderer::getQueueFamilyIndices() const
{
	if (_graphicsQueueIndex == _transferQueueIndex)
	{
		return std::vector<uint32_t>{_graphicsQueueIndex};
	}
	return std::vector<uint32_t>{_graphicsQueueIndex, _transferQueueIndex};
}

vk::CommandBuffer de::vulkan::renderer::beginSingleTimeTransferCommands()
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

void de::vulkan::renderer::submitSingleTimeTransferCommands(vk::CommandBuffer commandBuffer)
{
	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo().setCommandBuffers({1, &commandBuffer});

	submitSingleTimeTransferCommands({submitInfo});
}

void de::vulkan::renderer::submitSingleTimeTransferCommands(const std::vector<vk::SubmitInfo>& submits)
{
	_transferQueue.submit(submits, nullptr);
	_transferQueue.waitIdle();
}

void de::vulkan::renderer::createInstance()
{
	uint32_t instanceExtensionsCount{};
	char const* const* instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);

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
	const vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, instanceLayers.size(), instanceLayers.data(), instanceExtensionsCount, instanceExtensions);
	_instance = vk::createInstance(instanceCreateInfo);
}

void de::vulkan::renderer::createPhysicalDevice()
{
	const auto physicalDevices = _instance.enumeratePhysicalDevices();
	for (vk::PhysicalDevice physicalDevice : physicalDevices)
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

	if (!_physicalDevice)
	{
		throw std::runtime_error("No Vulkan supported GPU");
	}
}

void de::vulkan::renderer::createDevice()
{
	const auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
	queueCreateInfoList.reserve(queueFamilyProperties.size());

	for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		constexpr std::array<float, 1> priorities{1.F};
		queueCreateInfoList.emplace_back()
			.setQueueFamilyIndex(i)
			.setQueueCount(priorities.size())
			.setQueuePriorities(priorities);
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

void de::vulkan::renderer::createQueues()
{
	const auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
	const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
	for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
	{
		const auto queueFlags = queueFamilyProperties[i].queueFlags;
		if ((queueFlags & vk::QueueFlagBits::eGraphics) && (queueFlags & vk::QueueFlagBits::eTransfer))
		{
			_graphicsQueueIndex = i;
			_transferQueueIndex = i;
		}
	}
	_graphicsQueue = _device.getQueue(_graphicsQueueIndex, 0);
	_transferQueue = _device.getQueue(_transferQueueIndex, 0);
}

void de::vulkan::renderer::createCommandPools()
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

void de::vulkan::renderer::createBufferPools()
{
	constexpr auto vertIndxUsage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	constexpr auto vertIndxSize = 256 * 1024 * 1024;
	_bpVertIndx.allocate(utils::memory_property::device, vertIndxUsage, vertIndxSize);

	constexpr auto uniformsUsage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
	constexpr auto uniformsSize = 64 * 1024 * 1024;
	_bpUniforms.allocate(utils::memory_property::device, uniformsUsage, uniformsSize);

	constexpr auto transferUsage = vk::BufferUsageFlagBits::eTransferSrc;
	constexpr auto transferSize = 256 * 1024 * 1024;
	_bpTransfer.allocate(utils::memory_property::host, transferUsage, transferSize);
}

void de::vulkan::renderer::createCameraBuffer()
{
	_cameraDataBufferId = getUniformBufferPool().makeBuffer(sizeof(camera_data));
}

void de::vulkan::renderer::setCameraView(uint32_t viewIndex, const de::math::mat4& inView)
{
	getView(viewIndex)->setViewMatrix(inView);
}

void de::vulkan::renderer::updateCameraBuffer()
{
	constexpr auto size = sizeof(_cameraData);

	static const auto id = _bpTransfer.makeBuffer(size);

	auto region = _bpTransfer.map(id);
	std::memcpy(region, &_cameraData, size);
	_bpTransfer.unmap(id);

	const auto copyRegion = vk::BufferCopy(0, 0, size);
	de::vulkan::buffer::copyBuffer(_bpTransfer.getBuffer(id).get(), _bpUniforms.getBuffer(_cameraDataBufferId).get(), {copyRegion});
}