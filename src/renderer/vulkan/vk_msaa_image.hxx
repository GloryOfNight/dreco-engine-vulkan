#pragma once
#include "vk_texture_image.hxx"

class vk_msaa_image : public vk_texture_image
{
public:
	void create() override;

	void recreate();

protected:
    VkImageAspectFlags getImageAspectFlags() const override;

	VkImageUsageFlags getImageUsageFlags() const override;
};