#pragma once
#include "vk_device_memory.hxx"

#include <map>
#include <vulkan/vulkan.hpp>

class vk_buffer final
{
public:
	using unique = std::unique_ptr<vk_buffer>;
	using id = uint64_t;

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
	vk_buffer_pool() = default;
	vk_buffer_pool(vk_buffer_pool&) = delete;
	vk_buffer_pool(vk_buffer_pool&&) = default;
	~vk_buffer_pool() { destroy(); };

	void allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size);
	void destroy();

	[[nodiscard]] vk_buffer::id makeBuffer(vk::DeviceSize size);

	[[nodiscard]] void* map(vk_buffer::id id);
	void unmap(vk_buffer::id id);

	void freeBuffer(vk_buffer::id id);

	const vk_buffer& getBuffer(const vk_buffer::id id) const;

private:
	vk::DeviceSize findBufferSpace(vk::DeviceSize size, vk::DeviceSize allignment);

	vk_device_memory _deviceMemory;

	vk::DeviceSize _size;
	vk::BufferUsageFlags _usage;
	vk::MemoryPropertyFlags _memoryPropertyFlags;

	std::map<vk_buffer::id, vk_buffer::unique> _buffers;
	vk_buffer::id _totalBuffersCounter;
};