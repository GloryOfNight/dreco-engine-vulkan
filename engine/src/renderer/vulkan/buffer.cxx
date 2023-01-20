#include "buffer.hxx"

#include "renderer.hxx"
#include "utils.hxx"

#include <limits>

void de::vulkan::buffer::create(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	_size = size;

	const renderer* renderer = renderer::get();
	const auto device{renderer->getDevice()};
	const auto sharingMode{renderer->getSharingMode()};
	const auto queueIndexes{renderer->getQueueFamilyIndices()};

	const vk::BufferCreateInfo bufferCreateInfo =
		vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(usage)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueIndexes);

	_buffer = device.createBuffer(bufferCreateInfo);
}

void de::vulkan::buffer::bind(device_memory& deviceMemory, vk::DeviceSize offset)
{
	_offset = offset;
	renderer::get()->getDevice().bindBufferMemory(get(), deviceMemory.get(), _offset);
}

void de::vulkan::buffer::destroy()
{
	if (_buffer)
		renderer::get()->getDevice().destroyBuffer(_buffer);
	_buffer = nullptr;
}

vk::Buffer de::vulkan::buffer::get() const
{
	return _buffer;
}

vk::DeviceSize de::vulkan::buffer::getSize() const
{
	return _size;
}

vk::DeviceSize de::vulkan::buffer::getOffset() const
{
	return _offset;
}

vk::DeviceSize de::vulkan::buffer::getEnd() const
{
	return getSize() + getOffset();
}

void de::vulkan::buffer::copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions)
{
	renderer* renderer{renderer::get()};

	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();

	commandBuffer.copyBuffer(bufferSrc, bufferDst, bufferCopyRegions);
	commandBuffer.end();

	renderer->submitSingleTimeTransferCommands(commandBuffer);
	renderer->getDevice().freeCommandBuffers(renderer->getTransferCommandPool(), commandBuffer);
}

vk::CommandBuffer de::vulkan::buffer::copyBufferToImage(const buffer& buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height)
{
	renderer* renderer{renderer::get()};

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

	commandBuffer.copyBufferToImage(buffer.get(), image, imageLayout, copyRegion);
	commandBuffer.end();

	return commandBuffer;
}

void de::vulkan::buffer_pool::allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	_size = size;
	_usage = usage;
	_memoryPropertyFlags = memoryPropertyFlags;

	buffer tempBuffer;
	tempBuffer.create(_memoryPropertyFlags, _usage, _size);

	const auto memoryRequirements = renderer::get()->getDevice().getBufferMemoryRequirements(tempBuffer.get());
	_deviceMemory.allocate(memoryRequirements, _memoryPropertyFlags);
}

void de::vulkan::buffer_pool::destroy()
{
	_buffers.clear();
	_deviceMemory.free();
}

de::vulkan::buffer::id de::vulkan::buffer_pool::makeBuffer(vk::DeviceSize size)
{
	// emplacing ruins everything
	auto buffer = std::make_unique<de::vulkan::buffer>();
	buffer->create(_memoryPropertyFlags, _usage, size);

	const auto memoryRequirements = renderer::get()->getDevice().getBufferMemoryRequirements(buffer->get());
	const auto offset = findBufferSpace(size, memoryRequirements.alignment);

	if (std::numeric_limits<vk::DeviceSize>::max() == offset)
	{
		throw de::except::out_of_space();
		return std::numeric_limits<buffer::id>::max();
	}

	buffer->bind(_deviceMemory, offset);

	auto id = _buffers.emplace(_totalBuffersCounter++, std::move(buffer)).first->first;
	return id;
}

void* de::vulkan::buffer_pool::map(buffer::id id)
{
	const auto& buffer = _buffers.at(id);

	const vk::Device device{renderer::get()->getDevice()};
	return device.mapMemory(_deviceMemory.get(), buffer->getOffset(), buffer->getSize());
}

void de::vulkan::buffer_pool::unmap(buffer::id id)
{
	const vk::Device device{renderer::get()->getDevice()};
	device.unmapMemory(_deviceMemory.get());
}

void de::vulkan::buffer_pool::freeBuffer(buffer::id id)
{
	_buffers.erase(id);
}

const de::vulkan::buffer& de::vulkan::buffer_pool::getBuffer(buffer::id id) const
{
	return *_buffers.at(id);
}

vk::DeviceSize de::vulkan::buffer_pool::findBufferSpace(vk::DeviceSize size, vk::DeviceSize allignment)
{
	vk::DeviceSize offset = 0;

	for (const auto& pair : _buffers)
	{
		const auto& [buffer_id, buffer] = pair;

		if (offset && buffer->getEnd() != offset)
		{
			if (buffer->getOffset() - offset >= size + allignment)
			{
				break;
			}
		}
		offset = buffer->getEnd();

		auto allignBy = offset % allignment;
		offset = allignBy ? offset + allignment - allignBy : offset;
	}

	if ((offset == 0 && _buffers.size() != 0) || size > _size)
	{
		return std::numeric_limits<vk::DeviceSize>::max();
	}

	return offset;
}
