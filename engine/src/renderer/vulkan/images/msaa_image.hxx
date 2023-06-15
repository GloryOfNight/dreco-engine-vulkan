#pragma once

#include "renderer/vulkan/image.hxx"

namespace de::vulkan
{
	class vk_msaa_image final : public image
	{
	public:
		void create(uint32_t viewIndex);

		void recreate();

	protected:
		vk::ImageAspectFlags getImageAspectFlags() const override;

		vk::ImageUsageFlags getImageUsageFlags() const override;

		uint32_t _viewIndex;
	};
} // namespace de::vulkan