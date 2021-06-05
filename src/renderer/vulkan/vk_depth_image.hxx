#pragma once

#include "vk_texture_image.hxx"

class vk_depth_image : public vk_texture_image
{
public:
	void create();

	void recreate();

	VkFormat getFormat() const;

protected:
	VkImageAspectFlags getImageAspectFlags() const override;

	VkImageUsageFlags getImageUsageFlags() const override;

	VkFormat findDepthFormat() const;

	bool hasStencilComponent() const;

private:
	VkFormat _format{VK_FORMAT_UNDEFINED};
};