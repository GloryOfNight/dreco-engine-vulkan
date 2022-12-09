#pragma once
#include "device_memory.hxx"

#include <map>
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class buffer final
	{
	public:
		using unique = std::unique_ptr<buffer>;
		using id = uint64_t;

		buffer() = default;
		buffer(const buffer&) = delete;
		buffer(buffer&&) = default;
		~buffer() { destroy(); };

		void create(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size);
		void bind(device_memory& deviceMemory, vk::DeviceSize offset);
		void destroy();

		vk::Buffer get() const;
		vk::DeviceSize getSize() const;
		vk::DeviceSize getOffset() const;
		vk::DeviceSize getEnd() const;

		static void copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions);

		[[nodiscard]] static vk::CommandBuffer copyBufferToImage(const buffer& buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height);

	private:
		vk::Buffer _buffer{};
		vk::DeviceSize _size{};
		vk::DeviceSize _offset{};
	};

	class buffer_pool final
	{
	public:
		buffer_pool() = default;
		buffer_pool(buffer_pool&) = delete;
		buffer_pool(buffer_pool&&) = default;
		~buffer_pool() { destroy(); };

		void allocate(vk::MemoryPropertyFlags memoryPropertyFlags, vk::BufferUsageFlags usage, vk::DeviceSize size);
		void destroy();

		[[nodiscard]] buffer::id makeBuffer(vk::DeviceSize size);

		[[nodiscard]] void* map(buffer::id id);
		void unmap(buffer::id id);

		void freeBuffer(buffer::id id);

		const buffer& getBuffer(const buffer::id id) const;

	private:
		vk::DeviceSize findBufferSpace(vk::DeviceSize size, vk::DeviceSize allignment);

		device_memory _deviceMemory;

		vk::DeviceSize _size;
		vk::BufferUsageFlags _usage;
		vk::MemoryPropertyFlags _memoryPropertyFlags;

		std::map<buffer::id, buffer::unique> _buffers;
		buffer::id _totalBuffersCounter;
	};
} // namespace de::vulkan