#include "depth_image.hxx"

#include "renderer/vulkan/renderer.hxx"
#include "renderer/vulkan/utils.hxx"

void de::vulkan::vk_depth_image::create()
{
	renderer* renderer{renderer::get()};
	const vk::Device device = renderer->getDevice();

	const vk::Extent2D extent = renderer->getCurrentExtent();

	_format = findSupportedDepthFormat();

	createImage(device, _format, extent.width, extent.height, renderer->getSettings().getPrefferedSampleCount());

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, utils::memory_property::device);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, _format);

	image_transition_layout_info transitionImageLayoutInfo;
	transitionImageLayoutInfo._image = _image;
	transitionImageLayoutInfo._format = _format;
	transitionImageLayoutInfo._layoutOld = vk::ImageLayout::eUndefined;
	transitionImageLayoutInfo._layoutNew = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	transitionImageLayoutInfo._accessFlagsSrc = vk::AccessFlagBits();
	transitionImageLayoutInfo._accessFlagsDst = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	transitionImageLayoutInfo._pipelineStageFlagsSrc = vk::PipelineStageFlagBits::eTopOfPipe;
	transitionImageLayoutInfo._pipelineStageFlagsDst = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	transitionImageLayoutInfo._imageAspectFlags = getImageAspectFlags();

	vk::CommandBuffer commandBuffer = transitionImageLayout(transitionImageLayoutInfo);
	renderer->submitSingleTimeTransferCommands(commandBuffer);

	device.freeCommandBuffers(renderer->getTransferCommandPool(), commandBuffer);
}

void de::vulkan::vk_depth_image::recreate()
{
	destroy();
	create();
}

vk::Format de::vulkan::vk_depth_image::getFormat() const
{
	return _format;
}

vk::ImageAspectFlags de::vulkan::vk_depth_image::getImageAspectFlags() const
{
	return hasStencilComponent() ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;
}

vk::ImageUsageFlags de::vulkan::vk_depth_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eDepthStencilAttachment;
}

vk::Format de::vulkan::vk_depth_image::findSupportedDepthFormat() const
{
	const vk::PhysicalDevice physicalDevice = renderer::get()->getPhysicalDevice();
	utils::find_supported_format_info findSupportedFormatInfo{};
	findSupportedFormatInfo._formatCandidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	findSupportedFormatInfo._imageTiling = vk::ImageTiling::eOptimal;
	findSupportedFormatInfo._formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

	return utils::findSupportedFormat(physicalDevice, findSupportedFormatInfo);
}

bool de::vulkan::vk_depth_image::hasStencilComponent() const
{
	return _format == vk::Format::eD32SfloatS8Uint || _format == vk::Format::eD24UnormS8Uint;
}