#include "vk_buffer.hxx"

#include "vk_exceptions.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

#include <limits>

void vk_buffer::create(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	_size = size;

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
}

void vk_buffer::bind(vk_device_memory& deviceMemory, vk::DeviceSize offset)
{
	_offset = offset;
	vk_renderer::get()->getDevice().bindBufferMemory(get(), deviceMemory.get(), _offset);
}

void vk_buffer::destroy()
{
	if (_buffer)
		vk_renderer::get()->getDevice().destroyBuffer(_buffer);
	_buffer = nullptr;
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

vk::DeviceSize vk_buffer::getEnd() const
{
	return getSize() + getOffset();
}

void vk_buffer::copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions)
{
	vk_renderer* renderer{vk_renderer::get()};

	vk::CommandBuffer commandBuffer = renderer->beginSingleTimeTransferCommands();

	commandBuffer.copyBuffer(bufferSrc, bufferDst, bufferCopyRegions);
	commandBuffer.end();

	renderer->submitSingleTimeTransferCommands(commandBuffer);
}

vk::CommandBuffer vk_buffer::copyBufferToImage(const vk_buffer& buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height)
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

	commandBuffer.copyBufferToImage(buffer.get(), image, imageLayout, copyRegion);
	commandBuffer.end();

	return commandBuffer;
}

void vk_buffer_pool::allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	_size = size;
	_usage = usage;
	_memoryPropertyFlags = memoryPropertyFlags;

	vk_buffer tempBuffer;
	tempBuffer.create(_memoryPropertyFlags, _usage, _size);

	const auto memoryRequirements = vk_renderer::get()->getDevice().getBufferMemoryRequirements(tempBuffer.get());
	_deviceMemory.allocate(memoryRequirements, _memoryPropertyFlags);
}

void vk_buffer_pool::destroy()
{
	_buffers.clear();
	_deviceMemory.free();
}

vk_buffer::id vk_buffer_pool::makeBuffer(vk::DeviceSize size)
{
	// emplacing ruins everything
	auto buffer = std::make_unique<vk_buffer>();
	buffer->create(_memoryPropertyFlags, _usage, size);

	const auto memoryRequirements = vk_renderer::get()->getDevice().getBufferMemoryRequirements(buffer->get());
	const auto offset = findBufferSpace(size, memoryRequirements.alignment);

	if (std::numeric_limits<vk::DeviceSize>::max() == offset)
	{
		throw vk_except::out_of_space();
		return std::numeric_limits<vk_buffer::id>::max();
	}

	buffer->bind(_deviceMemory, offset);

	auto id = _buffers.emplace(_totalBuffersCounter++, std::move(buffer)).first->first;
	return id;
}

void* vk_buffer_pool::map(vk_buffer::id id)
{
	const auto& buffer = _buffers.at(id);

	const vk::Device device{vk_renderer::get()->getDevice()};
	return device.mapMemory(_deviceMemory.get(), buffer->getOffset(), buffer->getSize());
}

void vk_buffer_pool::unmap(vk_buffer::id id)
{
	const vk::Device device{vk_renderer::get()->getDevice()};
	device.unmapMemory(_deviceMemory.get());
}

void vk_buffer_pool::freeBuffer(vk_buffer::id id)
{
	_buffers.erase(id);
}

const vk_buffer& vk_buffer_pool::getBuffer(vk_buffer::id id) const
{
	return *_buffers.at(id);
}

vk::DeviceSize vk_buffer_pool::findBufferSpace(vk::DeviceSize size, vk::DeviceSize allignment)
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
