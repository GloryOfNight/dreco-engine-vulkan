#pragma once

#include "core/containers/gltf/image.hxx"
#include "renderer/vulkan/image.hxx"

namespace de::vulkan
{
	class texture_image final : public image
	{
	public:
		texture_image() = default;
		texture_image(const texture_image&) = delete;
		texture_image(texture_image&&) = delete;
		virtual ~texture_image() { destroy(); };

		void create(const de::gltf::image& image);

		void destroy() override;

		vk::Sampler getSampler() const;

		bool isValid() const;

	protected:
		virtual vk::ImageAspectFlags getImageAspectFlags() const override;

		virtual vk::ImageUsageFlags getImageUsageFlags() const override;

		void createSampler(const vk::Device device);

		vk::Sampler _sampler;
	};
} // namespace de::vulkan