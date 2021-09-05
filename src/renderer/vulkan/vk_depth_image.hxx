#pragma once

#include "vk_image.hxx"

class vk_depth_image final : public vk_image
{
public:
	void create() override;

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