#pragma once
#include "vk_device_memory.hxx"

#include <map>
#include <vulkan/vulkan.hpp>

class vk_buffer final
{
public:
	using unique = std::unique_ptr<vk_buffer>;

	vk_buffer() = default;
	vk_buffer(const vk_buffer&) = delete;
	vk_buffer(vk_buffer&&) = default;
	~vk_buffer() { destroy(); };

	void create(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size);
	void bind(vk_device_memory& deviceMemory, vk::DeviceSize offset);
	void destroy();

	vk::Buffer get() const;
	vk::DeviceSize getSize() const;
	vk::DeviceSize getOffset() const;
	vk::DeviceSize getEnd() const;

	static void copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions);

	[[nodiscard]] static vk::CommandBuffer copyBufferToImage(const vk_buffer& buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height);

private:
	vk::Buffer _buffer{};
	vk::DeviceSize _size{};
	vk::DeviceSize _offset{};
};

class vk_buffer_pool final
{
public:
	using buffer_id = uint64_t;

	vk_buffer_pool() = default;
	vk_buffer_pool(vk_buffer_pool&) = delete;
	vk_buffer_pool(vk_buffer_pool&&) = default;
	~vk_buffer_pool() { destroy(); };

	void allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size);
	void destroy();

	[[nodiscard]] buffer_id makeBuffer(vk::DeviceSize size);

	[[nodiscard]] void* map(buffer_id id);
	void unmap(buffer_id id);

	void freeBuffer(buffer_id id);

	const vk_buffer& getBuffer(const buffer_id id) const;

private:
	vk::DeviceSize findBufferSpace(vk::DeviceSize size, vk::DeviceSize allignment);
	
	vk_device_memory _deviceMemory;

	vk::DeviceSize _size;
	vk::BufferUsageFlags _usage;
	vk::MemoryPropertyFlags _memoryPropertyFlags;

	std::map<buffer_id, vk_buffer::unique> _buffers;
	buffer_id _totalBuffersCounter;
};

class vk_buffer_region final
{
public:
	vk_buffer_region() = default;
	vk_buffer_region(const vk_buffer& buffer, const vk::DeviceSize size, const vk::DeviceSize offset = 0)
		: _buffer{&buffer}
		, _size{size}
		, _offset{offset}
	{
	}
	vk_buffer_region(const vk_buffer& buffer)
		: vk_buffer_region(buffer, buffer.getSize(), buffer.getOffset())
	{
	}

	const vk_buffer& getBuffer() const { return *_buffer; };
	vk::DeviceSize getSize() const { return _size; };
	vk::DeviceSize getOffset() const { return _offset; };

private:
	const vk_buffer* _buffer{};
	vk::DeviceSize _size{};
	vk::DeviceSize _offset{};
};