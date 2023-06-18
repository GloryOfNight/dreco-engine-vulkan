#include "msaa_image.hxx"

#include "renderer/vulkan/renderer.hxx"
#include "renderer/vulkan/utils.hxx"

void de::vulkan::vk_msaa_image::create(uint32_t viewIndex)
{
	const renderer* renderer{renderer::get()};
	const vk::Device device = renderer->getDevice();

	_viewIndex = viewIndex;

	auto view = renderer->getView(viewIndex);
	const auto format = view->getFormat();
	const auto extent = view->getCurrentExtent();
	const auto sampleCount = view->getSettings().getSampleCount();

	createImage(device, format, extent.width, extent.height, sampleCount);

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, utils::memory_property::device);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, format);
}

void de::vulkan::vk_msaa_image::recreate()
{
	destroy();
	create(_viewIndex);
}

vk::ImageAspectFlags de::vulkan::vk_msaa_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlagBits::eColor;
}

vk::ImageUsageFlags de::vulkan::vk_msaa_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
}