#pragma once
#include "core/containers/gltf/model.hxx"
#include "images/vk_depth_image.hxx"
#include "images/vk_msaa_image.hxx"
#include "images/vk_texture_image.hxx"
#include "renderer/shader_types/camera_data.hxx"

#include "vk_buffer.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_queue_family.hxx"
#include "vk_scene.hxx"
#include "vk_settings.hxx"

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

struct SDL_Window;
class engine;
class vk_mesh;

class vk_renderer final
{
public:
	vk_renderer() = default;
	vk_renderer(const vk_renderer&) = delete;
	vk_renderer(vk_renderer&&) = delete;
	~vk_renderer();

	vk_renderer& operator=(vk_renderer&) = delete;
	vk_renderer& operator=(vk_renderer&&) = delete;

	static vk_renderer* get();

	static bool isSupported();

	void init();

	void exit();

	void tick(double deltaTime);

	void setCameraData(const mat4& inView, const mat4 inProj);

	void loadModel(const gltf::model& scn);

	vk_shader::shared loadShader(const std::string_view& path);

	uint32_t getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch = nullptr);

	uint32_t getImageCount() const;

	vk::RenderPass getRenderPass() const { return _renderPass; };

	vk::CommandPool getTransferCommandPool() const { return _transferCommandPool; };

	SDL_Window* getWindow() const { return _window; };

	uint32_t getWindowId() const { return _windowId; };

	vk::Extent2D getCurrentExtent() const { return _currentExtent; };

	vk::Device getDevice() const { return _device; }

	vk::SurfaceKHR getSurface() const { return _surface; }

	vk::PhysicalDevice getPhysicalDevice() const { return _physicalDevice; }

	const vk_queue_family& getQueueFamily() const { return _queueFamily; }
	vk_queue_family& getQueueFamily() { return _queueFamily; }

	const vk_settings& getSettings() const { return _settings; }
	vk_settings& getSettings() { return _settings; }

	const std::vector<std::unique_ptr<vk_scene>>& getScenes() const { return _scenes; };
	std::vector<std::unique_ptr<vk_scene>>& getScenes() { return _scenes; };

	const vk_texture_image& getTextureImagePlaceholder() const { return _placeholderTextureImage; }

	const vk_buffer_pool& getVertIndxBufferPool() const { return _bpVertIndx; };
	vk_buffer_pool& getVertIndxBufferPool() { return _bpVertIndx; };

	const vk_buffer_pool& getUniformBufferPool() const { return _bpUniforms; };
	vk_buffer_pool& getUniformBufferPool() { return _bpUniforms; };

	const vk_buffer_pool& getTransferBufferPool() const { return _bpTransfer; };
	vk_buffer_pool& getTransferBufferPool() { return _bpTransfer; };

	const vk_buffer& getCameraDataBuffer() const { return getUniformBufferPool().getBuffer(_cameraDataBufferId); };

	vk::CommandBuffer beginSingleTimeTransferCommands();

	void submitSingleTimeTransferCommands(vk::CommandBuffer commandBuffer);

	void submitSingleTimeTransferCommands(const std::vector<vk::SubmitInfo>& submits);

	void applySettings();

protected:
	void updateCameraBuffer();

	void drawFrame();

	bool updateExtent();

	void createWindow();

	void createInstance();

	void createSurface();

	void createPhysicalDevice();

	void createDevice();

	void createSwapchain();

	void createImageViews();

	void createRenderPass();

	void createFramebuffers();

	void createCommandPools();

	void createPrimaryCommandBuffers();

	void createFences();

	void createSemaphores();

	void createBufferPools();

	void createCameraBuffer();

	void cleanupSwapchain(vk::SwapchainKHR swapchain);

	void recreateSwapchain();

	vk::CommandBuffer prepareCommandBuffer(uint32_t imageIndex);

private:
	uint32_t _apiVersion{};

	vk_texture_image _placeholderTextureImage;

	std::vector<std::unique_ptr<vk_scene>> _scenes;

	SDL_Window* _window{};

	uint32_t _windowId{};

	vk::Extent2D _currentExtent;

	vk::SurfaceKHR _surface;

	vk::Instance _instance;

	vk::PhysicalDevice _physicalDevice;

	vk::Device _device;

	vk::Queue _graphicsQueue, _presentQueue, _transferQueue;

	vk_queue_family _queueFamily;

	vk_settings _settings;

	vk_msaa_image _msaaImage;

	vk_depth_image _depthImage;

	camera_data _cameraData;
	vk_buffer_pool::buffer_id _cameraDataBufferId{std::numeric_limits<vk_buffer_pool::buffer_id>::max()};

	vk_buffer_pool _bpVertIndx;
	vk_buffer_pool _bpUniforms;
	vk_buffer_pool _bpTransfer;

	std::map<std::string, vk_shader::shared> _shaders;

	vk::SwapchainKHR _swapchain;

	std::vector<vk::ImageView> _swapchainImageViews;

	vk::RenderPass _renderPass;

	std::vector<vk::Framebuffer> _framebuffers;

	std::vector<vk::CommandPool> _graphicsCommandPools;
	std::vector<vk::CommandBuffer> _graphicsCommandBuffers;

	vk::CommandPool _transferCommandPool;

	std::vector<vk::Fence> _submitQueueFences;

	vk::Semaphore _semaphoreImageAvaible, _semaphoreRenderFinished;
};