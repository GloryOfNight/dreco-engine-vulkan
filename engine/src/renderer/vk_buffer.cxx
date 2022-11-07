#include "vk_buffer.hxx"

#include "vk_renderer.hxx"
#include "vk_utils.hxx"

void vk_buffer::allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	const vk_renderer* renderer = vk_renderer::get();
	const auto device{renderer->getDevice()};
	const auto& queueFamily{renderer->getQueueFamily()};
	const auto sharingMode{queueFamily.getSharingMode()};
	const auto queueIndexes{queueFamily.getUniqueQueueIndexes(sharingMode)};

	const vk::BufferCreateInfo bufferCreateInfo =
		vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(usage)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueIndexes);

	_buffer = device.createBuffer(bufferCreateInfo);

	const vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(get());
	_deviceMemory.allocate(memoryRequirements, memoryPropertyFlags);

	device.bindBufferMemory(get(), _deviceMemory.get(), getOffset());

	_size = size;
}

void vk_buffer::destroy()
{
	auto device = vk_renderer::get()->getDevice();

	if (_buffer)
		device.destroyBuffer(_buffer);
	_buffer = nullptr;

	_deviceMemory.free();
}

vk::Buffer vk_buffer::get() const
{
	return _buffer;
}

vk::DeviceSize vk_buffer::getSize() const
{
	return _size;
}

vk::DeviceSize vk_buffer::getOffset() const
{
	return _offset;
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