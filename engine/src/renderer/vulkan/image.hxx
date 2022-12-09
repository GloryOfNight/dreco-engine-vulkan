#pragma once
#include "device_memory.hxx"

#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	struct image_transition_layout_info
	{
		vk::Image _image;
		vk::Format _format;
		vk::ImageLayout _layoutOld, _layoutNew;
		vk::AccessFlags _accessFlagsSrc, _accessFlagsDst;
		vk::PipelineStageFlags _pipelineStageFlagsSrc, _pipelineStageFlagsDst;
		vk::ImageAspectFlags _imageAspectFlags;
	};

	class image
	{
	public:
		image() = default;
		image(const image&) = delete;
		image(image&&) = default;
		virtual ~image() { destroy(); };

		virtual void destroy();

		vk::Image getImage() const { return _image; };

		vk::ImageView getImageView() const { return _imageView; };

		const device_memory& getDeviceMemory() const { return _deviceMemory; };

		[[nodiscard]] static VkCommandBuffer transitionImageLayout(const image_transition_layout_info& info);

	protected:
		virtual vk::ImageAspectFlags getImageAspectFlags() const;

		virtual vk::ImageUsageFlags getImageUsageFlags() const;

		void createImage(const vk::Device device, const vk::Format format, const uint32_t width, const uint32_t height, const vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

		void bindToMemory(const vk::Device device, const vk::DeviceMemory deviceMemory, const vk::DeviceSize memoryOffset);

		void createImageView(const vk::Device device, const vk::Format format);

		vk::Image _image;

		vk::ImageView _imageView;

		device_memory _deviceMemory;
	};
} // namespace de::vulkan