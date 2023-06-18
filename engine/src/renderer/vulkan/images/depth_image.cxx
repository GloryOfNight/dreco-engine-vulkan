#include "depth_image.hxx"

#include "renderer/vulkan/renderer.hxx"
#include "renderer/vulkan/utils.hxx"

void de::vulkan::vk_depth_image::create(uint32_t viewIndex)
{
	renderer* renderer{renderer::get()};
	const vk::Device device = renderer->getDevice();

	_format = utils::findSupportedDepthFormat(renderer->getPhysicalDevice());

	_viewIndex = viewIndex;

	const auto view = renderer->getView(viewIndex);
	const auto extent = view->getCurrentExtent();
	const auto sampleCount = view->getSettings().getSampleCount();

	createImage(device, _format, extent.width, extent.height, sampleCount);

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
	create(_viewIndex);
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

bool de::vulkan::vk_depth_image::hasStencilComponent() const
{
	return _format == vk::Format::eD32SfloatS8Uint || _format == vk::Format::eD24UnormS8Uint;
}