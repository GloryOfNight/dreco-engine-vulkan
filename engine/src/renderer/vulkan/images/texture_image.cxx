#include "texture_image.hxx"

#include "renderer/vulkan/renderer.hxx"
#include "renderer/vulkan/utils.hxx"

#include "dreco.hxx"

void de::vulkan::texture_image::create(const de::gltf::image& image)
{
	renderer* renderer{renderer::get()};
	const vk::Device device = renderer->getDevice();

	const vk::Format format = vk::Format::eR8G8B8A8Srgb;
	createImage(device, format, image._width, image._height);

	const vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
	_deviceMemory.allocate(memoryRequirements, utils::memory_property::device);

	bindToMemory(device, _deviceMemory.get(), 0);

	createImageView(device, format);
	createSampler(device);

	auto& bpTransfer = renderer->getTransferBufferPool();
	const auto transferBufferId = bpTransfer.makeBuffer(memoryRequirements.size);
	auto region = bpTransfer.map(transferBufferId);

	memcpy(region, image._pixels.data(), image._pixels.size());

	bpTransfer.unmap(transferBufferId);

	// clang-format off
	const std::array<vk::Semaphore, 2> semaphores =
		{
			device.createSemaphore(vk::SemaphoreCreateInfo()),
			device.createSemaphore(vk::SemaphoreCreateInfo())
		};
	// clang-format on

	std::array<vk::CommandBuffer, 3> commandBuffers;

	image_transition_layout_info transitionLayoutInfo{};
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

	commandBuffers[1] = de::vulkan::buffer::copyBufferToImage(
		bpTransfer.getBuffer(transferBufferId), _image,
		vk::ImageLayout::eTransferDstOptimal, image._width, image._height);

	transitionLayoutInfo._layoutOld = vk::ImageLayout::eTransferDstOptimal;
	transitionLayoutInfo._layoutNew = vk::ImageLayout::eShaderReadOnlyOptimal;
	transitionLayoutInfo._accessFlagsSrc = vk::AccessFlagBits::eTransferWrite;
	transitionLayoutInfo._accessFlagsDst = vk::AccessFlagBits::eShaderRead;
	transitionLayoutInfo._pipelineStageFlagsSrc = vk::PipelineStageFlagBits::eTransfer;
	transitionLayoutInfo._pipelineStageFlagsDst = vk::PipelineStageFlagBits::eFragmentShader;

	commandBuffers[2] = transitionImageLayout(transitionLayoutInfo);

	const std::array<vk::PipelineStageFlags, 2> stageFlags{
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader};

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

	bpTransfer.freeBuffer(transferBufferId);
}

void de::vulkan::texture_image::destroy()
{
	image::destroy();
}

vk::Sampler de::vulkan::texture_image::getSampler() const
{
	return _sampler;
}

bool de::vulkan::texture_image::isValid() const
{
	return getImage() && getImageView() && getSampler();
}

vk::ImageAspectFlags de::vulkan::texture_image::getImageAspectFlags() const
{
	return vk::ImageAspectFlagBits::eColor;
}

vk::ImageUsageFlags de::vulkan::texture_image::getImageUsageFlags() const
{
	return vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
}