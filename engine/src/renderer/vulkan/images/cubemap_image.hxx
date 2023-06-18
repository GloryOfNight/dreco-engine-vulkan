#pragma once

#include "renderer/vulkan/image.hxx"

#include <array>
#include <string>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class cubemap_image : public image
	{
	public:
		void create(const std::array<std::string, 6>& cubeTexures);

		void destroy() override;

	protected:
		vk::ImageAspectFlags getImageAspectFlags() const override { return vk::ImageAspectFlagBits::eColor; }

		vk::ImageUsageFlags getImageUsageFlags() const override { return vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst; };

		virtual uint32_t getLayerCount() const override { return 6; }

		virtual vk::ImageCreateFlags getImageCreateFlags() const override { return vk::ImageCreateFlagBits::eCubeCompatible; }

		virtual vk::ImageViewType getImageViewType() const override { return vk::ImageViewType::eCube; }
	};
} // namespace de::vulkan