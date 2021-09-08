#pragma once

#include "core/containers/image.hxx"
#include "renderer/containers/material.hxx"
#include "renderer/containers/texture_data.hxx"

#include "vk_image.hxx"

#include <string_view>

class vk_texture_image : public vk_image
{
public:
	vk_texture_image();
	vk_texture_image(const vk_texture_image&) = delete;
	vk_texture_image(vk_texture_image&&) = delete;
	virtual ~vk_texture_image();

	void create() override;

	void create(const image& img);

	void create(const texture_data& textureData);

	void destroy() override;

	VkSampler getSampler() const;

protected:
	virtual VkImageAspectFlags getImageAspectFlags() const override;

	virtual VkImageUsageFlags getImageUsageFlags() const override;

	void createSampler(const VkDevice vkDevice);

	VkSampler _vkSampler;
};