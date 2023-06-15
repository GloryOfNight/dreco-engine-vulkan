#pragma once
#include "core/containers/gltf/model.hxx"
#include "images/depth_image.hxx"
#include "images/msaa_image.hxx"
#include "images/texture_image.hxx"
#include "renderer/shader_types/camera_data.hxx"

#include "buffer.hxx"
#include "material.hxx"
#include "scene.hxx"
#include "settings.hxx"
#include "view.hxx"

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

struct SDL_Window;

namespace de::vulkan
{
	class engine;
	class mesh;

	class renderer final
	{
	public:
		renderer() = default;
		renderer(const renderer&) = delete;
		renderer(renderer&&) = delete;
		~renderer();

		renderer& operator=(renderer&) = delete;
		renderer& operator=(renderer&&) = delete;

		static renderer* get();

		static bool isSupported();

		void init();

		void exit();

		void tick(double deltaTime);

		uint32_t addView(SDL_Window* window);
		void removeView(uint32_t viewIndex);

		material* getMaterial(const std::string_view& name) const;

		void setCameraView(const de::math::mat4& inView);

		void loadModel(const de::gltf::model& scn);

		shader::shared loadShader(const std::string_view& path);

		uint32_t getVersion(uint32_t& major, uint32_t& minor, uint32_t* patch = nullptr);

		uint32_t getImageCount() const;

		vk::SharingMode getSharingMode() const;

		std::vector<uint32_t> getQueueFamilyIndices() const;

		std::array<view::unique, 16>& getViews() { return _views; }

		view* getView(uint32_t index) const { return _views[index].get(); }

		uint32_t getCurrentDrawViewIndex() const { return _currentDrawViewIndex; }

		vk::Queue getGraphicsQueue() const { return _graphicsQueue; }

		vk::RenderPass getRenderPass() const { return _renderPass; }

		vk::CommandPool getTransferCommandPool() const { return _transferCommandPool; }

		SDL_Window* getWindow() const { return _window; }

		uint32_t getWindowId() const { return _windowId; }

		vk::Extent2D getCurrentExtent() const { return _currentExtent; }

		vk::Instance getInstance() { return _instance; }

		vk::PhysicalDevice getPhysicalDevice() const { return _physicalDevice; }

		vk::Device getDevice() const { return _device; }

		vk::SurfaceKHR getSurface() const { return _surface; }

		const settings& getSettings() const { return _settings; }
		settings& getSettings() { return _settings; }

		const std::vector<std::unique_ptr<scene>>& getScenes() const { return _scenes; }
		std::vector<std::unique_ptr<scene>>& getScenes() { return _scenes; }

		const texture_image& getTextureImagePlaceholder() const { return _placeholderTextureImage; }

		const de::vulkan::buffer_pool& getVertIndxBufferPool() const { return _bpVertIndx; }
		de::vulkan::buffer_pool& getVertIndxBufferPool() { return _bpVertIndx; }

		const de::vulkan::buffer_pool& getUniformBufferPool() const { return _bpUniforms; }
		de::vulkan::buffer_pool& getUniformBufferPool() { return _bpUniforms; }

		const de::vulkan::buffer_pool& getTransferBufferPool() const { return _bpTransfer; }
		de::vulkan::buffer_pool& getTransferBufferPool() { return _bpTransfer; }

		const de::vulkan::buffer& getCameraDataBuffer() const { return getUniformBufferPool().getBuffer(_cameraDataBufferId); }

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

		void createQueues();

		void createSwapchain();

		void createImageViews();

		void createRenderPass();

		void createFramebuffers();

		void createCommandPools();

		void createImageCommandBuffers();

		void createFences();

		void createSemaphores();

		void createBufferPools();

		void createCameraBuffer();

		void cleanupSwapchain(vk::SwapchainKHR swapchain);

		void recreateSwapchain();

		vk::CommandBuffer prepareCommandBuffer(uint32_t imageIndex);

	private:
		uint32_t _apiVersion{};

		texture_image _placeholderTextureImage;

		std::vector<std::unique_ptr<scene>> _scenes;

		SDL_Window* _window{};

		uint32_t _windowId{};

		std::array<view::unique, 16> _views;
		uint32_t _currentDrawViewIndex{};

		std::map<std::string, shader::shared> _shaders;

		std::map<std::string, material::unique> _materials;

		vk::Extent2D _currentExtent;

		vk::SurfaceKHR _surface;

		vk::Instance _instance;

		vk::PhysicalDevice _physicalDevice;

		vk::Device _device;

		uint32_t _graphicsQueueIndex, _transferQueueIndex;
		vk::Queue _graphicsQueue, _transferQueue;
		vk::CommandPool _graphicsCommandPool, _transferCommandPool;

		settings _settings;

		vk_msaa_image _msaaImage;

		vk_depth_image _depthImage;

		camera_data _cameraData;
		de::vulkan::buffer::id _cameraDataBufferId{std::numeric_limits<de::vulkan::buffer::id>::max()};

		de::vulkan::buffer_pool _bpVertIndx;
		de::vulkan::buffer_pool _bpUniforms;
		de::vulkan::buffer_pool _bpTransfer;

		vk::SwapchainKHR _swapchain;

		std::vector<vk::ImageView> _swapchainImageViews;
		std::vector<vk::CommandBuffer> _imageCommandBuffers;

		vk::RenderPass _renderPass;

		std::vector<vk::Framebuffer> _framebuffers;

		std::vector<vk::Fence> _submitQueueFences;

		vk::Semaphore _semaphoreImageAvailable, _semaphoreRenderFinished;
	};
} // namespace de::vulkan