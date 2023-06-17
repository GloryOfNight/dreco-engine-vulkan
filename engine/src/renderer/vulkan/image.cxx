#include "image.hxx"

#include "renderer.hxx"
#include "utils.hxx"

void de::vulkan::image::destroy()
{
	if (_sampler)
	{
		const vk::Device device = renderer::get()->getDevice();
		device.destroySampler(_sampler);
		_sampler = nullptr;
	}
	if (_imageView)
	{
		const vk::Device device = renderer::get()->getDevice();
		device.destroyImageView(_imageView);
		_imageView = nullptr;
	}
	if (_image)
	{
		const vk::Device device = renderer::get()->getDevice();
		device.destroyImage(_image);
		_image = nullptr;
	}
	_deviceMemory.free();
}

void de::vulkan::image::createImage(const vk::Device device, const vk::Format format, const uint32_t width, const uint32_t height, const vk::SampleCountFlagBits samples)
{
	const auto renderer{renderer::get()};
	const auto sharingMode{renderer->getSharingMode()};
	const auto queueIndexes{renderer->getQueueFamilyIndices()};

	const vk::ImageCreateInfo imageCreateInfo =
		vk::ImageCreateInfo()
			.setFlags(getImageCreateFlags())
			.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setExtent(vk::Extent3D(width, height, 1))
			.setMipLevels(1)
			.setArrayLayers(getLayerCount())
			.setSamples(samples)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(getImageUsageFlags())
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(queueIndexes)
			.setSharingMode(sharingMode);

	_image = device.createImage(imageCreateInfo);
}

void de::vulkan::image::bindToMemory(const vk::Device device, const vk::DeviceMemory deviceMemory, const vk::DeviceSize memoryOffset)
{
	device.bindImageMemory(_image, deviceMemory, memoryOffset);
}

void de::vulkan::image::createImageView(const vk::Device device, const vk::Format format)
{
	const vk::ImageSubresourceRange imageSubresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(getImageAspectFlags())
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(getLayerCount())
			.setLevelCount(1);

	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setFlags({})
			.setImage(_image)
			.setFormat(format)
			.setViewType(getImageViewType())
			.setSubresourceRange(imageSubresourceRange);

	_imageView = device.createImageView(imageViewCreateInfo);
}

VkCommandBuffer de::vulkan::image::transitionImageLayout(const image_transition_layout_info& info)
{
	const auto renderer{renderer::get()};
	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();
	const vk::ImageSubresourceRange imageSubresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(info._imageAspectFlags)
			.setBaseMipLevel(0)
			.setBaseArrayLayer(0)
			.setLevelCount(1)
			.setLayerCount(getLayerCount());

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

void de::vulkan::image::createSampler(const vk::Device device)
{
	const vk::PhysicalDevice physicalDevice = renderer::get()->getPhysicalDevice();
	const vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
	const vk::PhysicalDeviceFeatures physicalDeviceFeatures = physicalDevice.getFeatures();

	const vk::SamplerCreateInfo samplerCreateInfo =
		vk::SamplerCreateInfo()
			.setFlags({})
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setMipLodBias(0.0F)
			.setAnisotropyEnable(physicalDeviceFeatures.samplerAnisotropy)
			.setMaxAnisotropy(physicalDeviceProperties.limits.maxSamplerAnisotropy)
			.setCompareEnable(VK_TRUE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMinLod(0.0F)
			.setMaxLod(0.0F)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE);

	_sampler = device.createSampler(samplerCreateInfo);
}