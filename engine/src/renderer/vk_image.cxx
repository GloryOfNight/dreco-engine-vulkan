#include "vk_image.hxx"

#include "vk_renderer.hxx"
#include "vk_utils.hxx"

void vk_image::destroy()
{
	if (_imageView)
	{
		const vk::Device device = vk_renderer::get()->getDevice();
		device.destroyImageView(_imageView);
		_imageView = nullptr;
	}
	if (_image)
	{
		const vk::Device device = vk_renderer::get()->getDevice();
		device.destroyImage(_image);
		_image = nullptr;
	}
	_deviceMemory.free();
}

vk::ImageAspectFlags vk_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlags();
}

vk::ImageUsageFlags vk_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlags();
}

void vk_image::createImage(const vk::Device device, const vk::Format format, const uint32_t width, const uint32_t height, const vk::SampleCountFlagBits samples)
{
	const auto renderer{vk_renderer::get()};
	const auto sharingMode{renderer->getSharingMode()};
	const auto queueIndexes{renderer->getQueueFamilyIndices()};

	const vk::ImageCreateInfo imageCreateInfo =
		vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setExtent(vk::Extent3D(width, height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(samples)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(getImageUsageFlags())
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(queueIndexes)
			.setSharingMode(sharingMode);

	_image = device.createImage(imageCreateInfo);
}

void vk_image::bindToMemory(const vk::Device device, const vk::DeviceMemory deviceMemory, const vk::DeviceSize memoryOffset)
{
	device.bindImageMemory(_image, deviceMemory, memoryOffset);
}

void vk_image::createImageView(const vk::Device device, const vk::Format format)
{
	const vk::ImageSubresourceRange imageSubresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(getImageAspectFlags())
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setFlags({})
			.setImage(_image)
			.setFormat(format)
			.setViewType(vk::ImageViewType::e2D)
			.setSubresourceRange(imageSubresourceRange);

	_imageView = device.createImageView(imageViewCreateInfo);
}

VkCommandBuffer vk_image::transitionImageLayout(const vk_image_transition_layout_info& info)
{
	const auto renderer{vk_renderer::get()};
	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();
	const vk::ImageSubresourceRange imageSubresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(info._imageAspectFlags)
			.setBaseMipLevel(0)
			.setBaseArrayLayer(0)
			.setLevelCount(1)
			.setLayerCount(1);

	const vk::ImageMemoryBarrier imageMemoryBarrier =
		vk::ImageMemoryBarrier()
			.setOldLayout(info._layoutOld)
			.setNewLayout(info._layoutNew)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(info._image)
			.setSubresourceRange(imageSubresourceRange)
			.setSrcAccessMask(info._accessFlagsSrc)
			.setDstAccessMask(info._accessFlagsDst);

	commandBuffer.pipelineBarrier(info._pipelineStageFlagsSrc, info._pipelineStageFlagsDst, {}, {}, {}, imageMemoryBarrier);
	commandBuffer.end();

	return commandBuffer;
}