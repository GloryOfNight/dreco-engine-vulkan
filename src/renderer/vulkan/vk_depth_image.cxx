#include "vk_depth_image.hxx"

#include "vk_renderer.hxx"
#include "vk_utils.hxx"

void vk_depth_image::create()
{
	vk_renderer* renderer{vk_renderer::get()};

	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();
	const vk::SurfaceKHR surface = renderer->getSurface();
	const VkExtent2D extent = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentExtent;

	_format = findSupportedDepthFormat();

	createImage(device, _format, extent.width, extent.height, renderer->getSettings().getPrefferedSampleCount());

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, vk_buffer::create_info::deviceMemoryPropertiesFlags);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, _format);

	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();

	vk_image_transition_layout_info transitionImageLayoutInfo;
	transitionImageLayoutInfo._image = _image;
	transitionImageLayoutInfo._format = _format;
	transitionImageLayoutInfo._layoutOld = vk::ImageLayout::eUndefined;
	transitionImageLayoutInfo._layoutNew = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	transitionImageLayoutInfo._accessFlagsSrc = vk::AccessFlagBits();
	transitionImageLayoutInfo._accessFlagsDst = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	transitionImageLayoutInfo._pipelineStageFlagsSrc = vk::PipelineStageFlagBits::eTopOfPipe;
	transitionImageLayoutInfo._pipelineStageFlagsDst = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	transitionImageLayoutInfo._imageAspectFlags = getImageAspectFlags();

	renderer->submitSingleTimeTransferCommands(commandBuffer);

	device.freeCommandBuffers(renderer->getTransferCommandPool(), std::array<vk::CommandBuffer, 1>{commandBuffer});
}

void vk_depth_image::recreate()
{
	destroy();
	create();
}

vk::Format vk_depth_image::getFormat() const
{
	return _format;
}

vk::ImageAspectFlags vk_depth_image::getImageAspectFlags() const
{
	return hasStencilComponent() ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;
}

vk::ImageUsageFlags vk_depth_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eDepthStencilAttachment;
}

vk::Format vk_depth_image::findSupportedDepthFormat() const
{
	const vk::PhysicalDevice physicalDevice = vk_renderer::get()->getPhysicalDevice();
	vk_utils::find_supported_format_info findSupportedFormatInfo{};
	findSupportedFormatInfo.formatCandidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	findSupportedFormatInfo.imageTiling = vk::ImageTiling::eOptimal;
	findSupportedFormatInfo.formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

	return vk_utils::findSupportedFormat(physicalDevice, findSupportedFormatInfo);
}

bool vk_depth_image::hasStencilComponent() const
{
	return _format == vk::Format::eD32SfloatS8Uint || _format == vk::Format::eD24UnormS8Uint;
}