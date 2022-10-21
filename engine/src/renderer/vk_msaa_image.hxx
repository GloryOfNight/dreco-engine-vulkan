#pragma once
#include "vk_image.hxx"

class vk_msaa_image : public vk_image
{
public:
	void create();

	void recreate();

protected:
	vk::ImageAspectFlags getImageAspectFlags() const override;

	vk::ImageUsageFlags getImageUsageFlags() const override;
};