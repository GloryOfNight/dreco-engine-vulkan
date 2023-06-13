#pragma once

#include "renderer/vulkan/image.hxx"

namespace de::vulkan
{
	class vk_depth_image final : public image
	{
	public:
		void create(vk::Extent2D extent);

		void recreate(vk::Extent2D extent);

		vk::Format getFormat() const;

	protected:
		vk::ImageAspectFlags getImageAspectFlags() const override;

		vk::ImageUsageFlags getImageUsageFlags() const override;

		vk::Format findSupportedDepthFormat() const;

		bool hasStencilComponent() const;

	private:
		vk::Format _format{VK_FORMAT_UNDEFINED};
	};
} // namespace de::vulkan