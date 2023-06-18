#pragma once

#include "images/depth_image.hxx"
#include "images/msaa_image.hxx"
#include "math/mat4.hxx"

#include "settings.hxx"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class view
	{
	public:
		using unique = std::unique_ptr<view>;

		view() = default;
		view(view&) = delete;
		view(view&&) = default;
		~view() { destroy(); }

		void init(vk::SurfaceKHR surface, uint32_t viewIndex);
		void recreateSwapchain();
		void destroy();

		bool isInitialized() const { return _surface != vk::SurfaceKHR(); }

		bool updateExtent(vk::PhysicalDevice physicalDevice);

		void setViewMatrix(const de::math::mat4& viewMatrix);
		const de::math::mat4& getViewMatrix() const { return _viewMatrix; };

		const settings& getSettings() const { return _settings; }
		void applySettings(settings&& newSettings);

		uint32_t acquireNextImageIndex();

		vk::CommandBuffer beginCommandBuffer(uint32_t imageIndex);
		void endCommandBuffer(vk::CommandBuffer commandBuffer);
		void submitCommandBuffer(uint32_t imageIndex, vk::CommandBuffer commandBuffer);

		vk::RenderPass getRenderPass() const { return _renderPass; };
		vk::Extent2D getCurrentExtent() const { return _currentExtent; };
		vk::SurfaceKHR getSurface() const { return _surface; }

		vk::SharingMode getSharingMode() const;

		uint32_t getImageCount() const;

		inline vk::Format getFormat() const { return vk::Format::eB8G8R8A8Srgb; }

	private:
		void createSwapchain(vk::PhysicalDevice physicalDevice, vk::Device device);

		void cleanupSwapchain(vk::Device device, vk::SwapchainKHR swapchain);

		void createImageViews(vk::Device device);

		void createRenderPass(vk::Device device);

		void createFramebuffers(vk::Device device);

		void createImageCommandBuffers(vk::Device device, vk::CommandPool transferCommandPool);

		void createFences(vk::Device device);

		void createSemaphores(vk::Device device);

		uint32_t _viewIndex;

		settings _settings;

		de::math::mat4 _viewMatrix;

		vk::SurfaceKHR _surface;

		vk::SwapchainKHR _swapchain;

		vk::Extent2D _currentExtent;

		vk_msaa_image _msaaImage;

		vk_depth_image _depthImage;

		std::vector<vk::ImageView> _swapchainImageViews;
		std::vector<vk::CommandBuffer> _imageCommandBuffers;

		vk::RenderPass _renderPass;

		std::vector<vk::Framebuffer> _framebuffers;

		std::vector<vk::Fence> _submitQueueFences;

		vk::Semaphore _semaphoreImageAvailable, _semaphoreRenderFinished;
	};
} // namespace de::vulkan
