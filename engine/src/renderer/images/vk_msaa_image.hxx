#pragma once

#include "renderer/vk_image.hxx"

class vk_msaa_image final : public vk_image
{
public:
	void create();

	void recreate();

protected:
	vk::ImageAspectFlags getImageAspectFlags() const override;

	vk::ImageUsageFlags getImageUsageFlags() const override;
};