#pragma once

#include "renderer/vulkan/image.hxx"

namespace de::vulkan
{
	class vk_msaa_image final : public image
	{
	public:
		void create();

		void recreate();

	protected:
		vk::ImageAspectFlags getImageAspectFlags() const override;

		vk::ImageUsageFlags getImageUsageFlags() const override;
	};
} // namespace de::vulkan