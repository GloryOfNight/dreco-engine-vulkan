#pragma once
#include "gltf/model.hxx"
#include "images/cubemap_image.hxx"
#include "images/depth_image.hxx"
#include "images/msaa_image.hxx"
#include "images/texture_image.hxx"
#include "renderer/shader_types/camera_data.hxx"

#include "buffer.hxx"
#include "material.hxx"
#include "scene.hxx"
#include "settings.hxx"
#include "skybox.hxx"
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

		const std::map<std::string, material::unique>& getMaterials() const { return _materials; }

		void setCameraView(uint32_t viewIndex, const de::math::mat4& inView);

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

		vk::CommandPool getTransferCommandPool() const { return _transferCommandPool; }

		vk::Instance getInstance() { return _instance; }

		vk::PhysicalDevice getPhysicalDevice() const { return _physicalDevice; }

		vk::Device getDevice() const { return _device; }

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

	protected:
		void updateCameraBuffer();

		void createInstance();

		void createPhysicalDevice();

		void createDevice();

		void createQueues();

		void createCommandPools();

		void createBufferPools();

		void createCameraBuffer();

		vk::CommandBuffer prepareCommandBuffer(uint32_t imageIndex);

	private:
		uint32_t _apiVersion{};

		texture_image _placeholderTextureImage;

		skybox _skybox;

		std::vector<std::unique_ptr<scene>> _scenes;

		std::array<view::unique, 16> _views;
		uint32_t _currentDrawViewIndex{};

		std::map<std::string, shader::shared> _shaders;

		std::map<std::string, material::unique> _materials;

		vk::Instance _instance;

		vk::PhysicalDevice _physicalDevice;

		vk::Device _device;

		uint32_t _graphicsQueueIndex, _transferQueueIndex;
		vk::Queue _graphicsQueue, _transferQueue;
		vk::CommandPool _graphicsCommandPool, _transferCommandPool;

		camera_data _cameraData;
		de::vulkan::buffer::id _cameraDataBufferId{std::numeric_limits<de::vulkan::buffer::id>::max()};

		de::vulkan::buffer_pool _bpVertIndx;
		de::vulkan::buffer_pool _bpUniforms;
		de::vulkan::buffer_pool _bpTransfer;
	};
} // namespace de::vulkan