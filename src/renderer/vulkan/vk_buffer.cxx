#include "vk_buffer.hxx"

#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

#include <cstring>

vk_buffer::vk_buffer()
	: _buffer{}
{
}

vk_buffer::~vk_buffer()
{
	destroy();
}

void vk_buffer::create(const create_info& createInfo)
{
	vk_renderer* renderer = vk_renderer::get();
	const vk::Device device = renderer->getDevice();

	const vk_queue_family& queueFamily{vk_renderer::get()->getQueueFamily()};
	const vk::SharingMode sharingMode = queueFamily.getSharingMode();

	std::vector<uint32_t> queueIndexes;
	if (sharingMode == vk::SharingMode::eConcurrent)
	{
		queueIndexes = queueFamily.getUniqueQueueIndexes();
	}

	const vk::BufferCreateInfo bufferCreateInfo =
		vk::BufferCreateInfo()
			.setSize(createInfo.size)
			.setUsage(createInfo.usage)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueIndexes);

	_buffer = device.createBuffer(bufferCreateInfo);

	const vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(_buffer);
	_deviceMemory.allocate(memoryRequirements, createInfo.memoryPropertiesFlags);

	device.bindBufferMemory(_buffer, _deviceMemory.get(), 0);
}

void vk_buffer::destroy()
{
	if (_buffer)
	{
		vk_renderer::get()->getDevice().destroyBuffer(_buffer);
		_buffer = nullptr;
	}
	_deviceMemory.free();
}

vk::Buffer vk_buffer::get() const
{
	return _buffer;
}

vk_device_memory& vk_buffer::getDeviceMemory()
{
	return _deviceMemory;
}

void vk_buffer::copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions)
{
	vk_renderer* renderer{vk_renderer::get()};

	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();

	commandBuffer.copyBuffer(bufferSrc, bufferDst, bufferCopyRegions);
	commandBuffer.end();

	renderer->submitSingleTimeTransferCommands(commandBuffer);
}

vk::CommandBuffer vk_buffer::copyBufferToImage(const vk::Buffer buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height)
{
	vk_renderer* renderer{vk_renderer::get()};

	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();

	const vk::ImageSubresourceLayers imageSubresourceLayers =
		vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

	const vk::BufferImageCopy copyRegion =
		vk::BufferImageCopy()
			.setBufferOffset(0)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource(imageSubresourceLayers)
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(width, height, 1));

	commandBuffer.copyBufferToImage(buffer, image, imageLayout, copyRegion);
	commandBuffer.end();

	return commandBuffer;
}