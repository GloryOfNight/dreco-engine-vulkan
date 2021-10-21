#include "vk_texture_image.hxx"

#include "core/utils/log.hxx"

#include "dreco.hxx"
#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_texture_image::vk_texture_image()
	: _sampler{}
{
}

vk_texture_image::~vk_texture_image()
{
	destroy();
}

void vk_texture_image::create()
{
	image_data imageData;
	if (!imageData.load(TEXTURE_PLACEHOLDER_URI))
	{
		DE_LOG(Critical, "Failed to load placeholder texture! Cannot proceed. . .");
		std::abort();
		return;
	}
	create(imageData);
}

void vk_texture_image::create(const image& img)
{
	image_data imageData;
	if (imageData.load(img._uri))
	{
		create(imageData);
	}
	else
	{
		DE_LOG(Error, "Failed to load texture from uri: %s; Loading default instead.", img._uri.data());
		create();
	}
}

void vk_texture_image::create(const image_data& textureData)
{
	if (!textureData.isLoaded())
	{
		DE_LOG(Error, "No valid texture data, using placeholder instead.");
		create();
		return;
	}

	uint16_t texWidth, texHeight;
	uint8_t texChannels;
	uint8_t* pixels{nullptr};
	textureData.getData(&pixels, &texWidth, &texHeight, &texChannels);

	vk_renderer* renderer{vk_renderer::get()};
	const vk::Device device = renderer->getDevice();

	const vk::Format format = vk::Format::eB8G8R8Unorm;
	createImage(device, format, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, format);
	createSampler(device);

	vk_buffer_create_info info;
	info.memory_properties_flags = vk_device_memory_properties::HOST;
	info.size = memoryRequirements.size;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	vk_buffer stagingBuffer;
	stagingBuffer.create(info);
	stagingBuffer.getDeviceMemory().map(pixels, texWidth * texHeight * 4);

	// clang-format off
	const std::array<vk::Semaphore, 2> semaphores =
		{
			device.createSemaphore(vk::SemaphoreCreateInfo()),
			device.createSemaphore(vk::SemaphoreCreateInfo())
		};
	// clang-format on

	std::array<vk::CommandBuffer, 3> commandBuffers;

	vk_image_transition_layout_info transitionLayoutInfo{};
	transitionLayoutInfo._image = _image;
	transitionLayoutInfo._format = format;
	transitionLayoutInfo._layoutOld = vk::ImageLayout::eUndefined;
	transitionLayoutInfo._layoutNew = vk::ImageLayout::eTransferDstOptimal;
	transitionLayoutInfo._accessFlagsSrc = vk::AccessFlagBits();
	transitionLayoutInfo._accessFlagsDst = vk::AccessFlagBits::eTransferWrite;
	transitionLayoutInfo._pipelineStageFlagsSrc = vk::PipelineStageFlagBits::eTopOfPipe;
	transitionLayoutInfo._pipelineStageFlagsDst = vk::PipelineStageFlagBits::eTransfer;
	transitionLayoutInfo._imageAspectFlags = getImageAspectFlags();

	commandBuffers[0] = transitionImageLayout(transitionLayoutInfo);

	commandBuffers[1] = vk_buffer::copyBufferToImage(stagingBuffer.get(), _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionLayoutInfo._layoutOld = vk::ImageLayout::eTransferDstOptimal;
	transitionLayoutInfo._layoutNew = vk::ImageLayout::eShaderReadOnlyOptimal;
	transitionLayoutInfo._accessFlagsSrc = vk::AccessFlagBits::eTransferWrite;
	transitionLayoutInfo._accessFlagsDst = vk::AccessFlagBits::eShaderRead;
	transitionLayoutInfo._pipelineStageFlagsSrc = vk::PipelineStageFlagBits::eTransfer;
	transitionLayoutInfo._pipelineStageFlagsDst = vk::PipelineStageFlagBits::eFragmentShader;

	commandBuffers[2] = transitionImageLayout(transitionLayoutInfo);

	std::array<vk::PipelineStageFlags, 2> stageFlags{
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader};
	std::vector<vk::SubmitInfo> submitInfos(3, vk::SubmitInfo());
	submitInfos[0].commandBufferCount = 1;
	submitInfos[0].pCommandBuffers = &commandBuffers[0];
	submitInfos[0].waitSemaphoreCount = 0;
	submitInfos[0].pWaitSemaphores = nullptr;
	submitInfos[0].signalSemaphoreCount = 1;
	submitInfos[0].pSignalSemaphores = &semaphores[0];
	submitInfos[0].pWaitDstStageMask = &stageFlags[0];

	submitInfos[1].commandBufferCount = 1;
	submitInfos[1].pCommandBuffers = &commandBuffers[1];
	submitInfos[1].waitSemaphoreCount = 1;
	submitInfos[1].pWaitSemaphores = semaphores.data();
	submitInfos[1].signalSemaphoreCount = 1;
	submitInfos[1].pSignalSemaphores = &semaphores[1];
	submitInfos[1].pWaitDstStageMask = &stageFlags[1];

	submitInfos[2].commandBufferCount = 1;
	submitInfos[2].pCommandBuffers = &commandBuffers[2];
	submitInfos[2].waitSemaphoreCount = 1;
	submitInfos[2].pWaitSemaphores = &semaphores[1];
	submitInfos[2].signalSemaphoreCount = 0;
	submitInfos[2].pSignalSemaphores = nullptr;
	submitInfos[2].pWaitDstStageMask = &stageFlags[1];

	renderer->submitSingleTimeTransferCommands(submitInfos);

	device.destroySemaphore(semaphores[0]);
	device.destroySemaphore(semaphores[1]);

	device.freeCommandBuffers(renderer->getTransferCommandPool(), commandBuffers);
}

void vk_texture_image::destroy()
{
	const vk::Device device = vk_renderer::get()->getDevice();
	if (_sampler)
	{
		device.destroySampler(_sampler);
		_sampler = nullptr;
	}
	vk_image::destroy();
}

vk::Sampler vk_texture_image::getSampler() const
{
	return _sampler;
}

bool vk_texture_image::isValid() const
{
	return getImage() && getImageView() && getSampler();
}

vk::ImageAspectFlags vk_texture_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlagBits::eColor;
}

vk::ImageUsageFlags vk_texture_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
}

void vk_texture_image::createSampler(const vk::Device device)
{
	const vk::PhysicalDevice physicalDevice = vk_renderer::get()->getPhysicalDevice();
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
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMinLod(0.0F)
			.setMaxLod(0.0F)
			.setBorderColor(vk::BorderColor::eIntOpaqueWhite)
			.setUnnormalizedCoordinates(VK_FALSE);

	_sampler = device.createSampler(samplerCreateInfo);
}
