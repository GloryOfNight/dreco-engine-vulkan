#pragma once

#include "vk_image.hxx"

class vk_depth_image final : public vk_image
{
public:
	void create();

	void recreate();

	vk::Format getFormat() const;

protected:
	vk::ImageAspectFlags getImageAspectFlags() const override;

	vk::ImageUsageFlags getImageUsageFlags() const override;

	vk::Format findSupportedDepthFormat() const;

	bool hasStencilComponent() const;

private:
	vk::Format _format{VK_FORMAT_UNDEFINED};
};