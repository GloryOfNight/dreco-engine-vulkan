#pragma once

#include "renderer/vulkan/image.hxx"

namespace de::vulkan
{
	class vk_depth_image final : public image
	{
	public:
		void create(uint32_t viewIndex);

		void recreate();

		vk::Format getFormat() const;

	protected:
		vk::ImageAspectFlags getImageAspectFlags() const override;

		vk::ImageUsageFlags getImageUsageFlags() const override;

		vk::Format findSupportedDepthFormat() const;

		bool hasStencilComponent() const;

	private:
		vk::Format _format{VK_FORMAT_UNDEFINED};

		uint32_t _viewIndex{};
	};
} // namespace de::vulkan