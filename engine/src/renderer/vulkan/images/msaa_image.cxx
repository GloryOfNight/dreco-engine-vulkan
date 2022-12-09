#include "msaa_image.hxx"

#include "renderer/vulkan/renderer.hxx"
#include "renderer/vulkan/utils.hxx"

void de::vulkan::vk_msaa_image::create()
{
	const renderer* renderer{renderer::get()};
	const vk::Device device = renderer->getDevice();

	const vk::Extent2D extent = renderer->getCurrentExtent();
	const vk::Format format = renderer->getSettings().getSurfaceFormat().format;
	const vk::SampleCountFlagBits samples = renderer->getSettings().getPrefferedSampleCount();

	createImage(device, format, extent.width, extent.height, samples);

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, utils::memory_property::device);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, format);
}

void de::vulkan::vk_msaa_image::recreate()
{
	destroy();
	create();
}

vk::ImageAspectFlags de::vulkan::vk_msaa_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlagBits::eColor;
}

vk::ImageUsageFlags de::vulkan::vk_msaa_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
}