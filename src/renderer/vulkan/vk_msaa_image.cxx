#include "vk_msaa_image.hxx"

#include "vk_renderer.hxx"

void vk_msaa_image::create()
{
	vk_renderer* renderer{vk_renderer::get()};
	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();
	const vk::SurfaceKHR surface = renderer->getSurface();
	
	const vk::Extent2D extent = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentExtent;
	const vk::Format format = renderer->getSettings().getSurfaceFormat().format;
	const vk::SampleCountFlagBits samples = renderer->getSettings().getPrefferedSampleCount();

	createImage(device, format, extent.width, extent.height, samples);

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, vk_buffer::create_info::deviceMemoryPropertiesFlags);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, format);
}

void vk_msaa_image::recreate() 
{
	destroy();
	create();
}

vk::ImageAspectFlags vk_msaa_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlagBits::eColor;
}

vk::ImageUsageFlags vk_msaa_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
}