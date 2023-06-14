#pragma once

#include "images/depth_image.hxx"
#include "images/msaa_image.hxx"

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

		void init(vk::SurfaceKHR surface);
		void recreateSwapchain();
		void destroy();

		bool isInitialized() const { return _surface != vk::SurfaceKHR(); }

		bool updateExtent(vk::PhysicalDevice physicalDevice);

		uint32_t acquireNextImageIndex();

		vk::CommandBuffer beginCommandBuffer(uint32_t imageIndex);
		void endCommandBuffer(vk::CommandBuffer commandBuffer);
		void submitCommandBuffer(uint32_t imageIndex, vk::CommandBuffer commandBuffer);

		vk::RenderPass getRenderPass() const { return _renderPass; };
		vk::Extent2D getCurrentExtent() const { return _currentExtent; };
		vk::SurfaceKHR getSurface() const { return _surface; }

		vk::SharingMode getSharingMode() const;

		uint32_t getImageCount() const;

	private:
		void createSwapchain(vk::PhysicalDevice physicalDevice, vk::Device device);

		void cleanupSwapchain(vk::Device device, vk::SwapchainKHR swapchain);

		void createImageViews(vk::Device device);

		void createRenderPass(vk::Device device);

		void createFramebuffers(vk::Device device);

		void createImageCommandBuffers(vk::Device device, vk::CommandPool transferCommandPool);

		void createFences(vk::Device device);

		void createSemaphores(vk::Device device);

		vk::SurfaceKHR _surface;

		vk::SurfaceFormatKHR _surfaceFormat;

		vk::PresentModeKHR _presentMode;

		vk::SampleCountFlagBits _maxSampleCount{vk::SampleCountFlagBits::e1};

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
