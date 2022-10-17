#pragma once

#include "core/containers/gltf/image.hxx"
#include "core/containers/image_data.hxx"

#include "vk_image.hxx"

#include <string_view>

class vk_texture_image : public vk_image
{
public:
	vk_texture_image() = default;
	vk_texture_image(const vk_texture_image&) = delete;
	vk_texture_image(vk_texture_image&&) = delete;
	virtual ~vk_texture_image() { destroy(); };

	void create(const image_data& textureData);

	void destroy() override;

	vk::Sampler getSampler() const;

	bool isValid() const;

protected:
	virtual vk::ImageAspectFlags getImageAspectFlags() const override;

	virtual vk::ImageUsageFlags getImageUsageFlags() const override;

	void createSampler(const vk::Device device);

	vk::Sampler _sampler;
};