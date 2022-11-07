#pragma once
#include "vk_device_memory.hxx"

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_device;
class vk_physical_device;
class vk_queue_family;

class vk_buffer final
{
public:
	struct create_info
	{
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryPropertiesFlags;
		vk::DeviceSize size;

		static inline vk::MemoryPropertyFlags deviceMemoryPropertiesFlags =
			vk::MemoryPropertyFlagBits::eDeviceLocal;

		static inline vk::MemoryPropertyFlags hostMemoryPropertiesFlags =
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	};

	vk_buffer() = default;
	vk_buffer(const vk_buffer&) = delete;
	vk_buffer(vk_buffer&&) = default;
	~vk_buffer() { destroy(); };

	void create(const create_info& createInfo);

	void destroy();

	vk::Buffer get() const;

	size_t getSize() const { return _size; };

	inline size_t getOffset() const { return 0; };

	vk_device_memory& getDeviceMemory();

	static void copyBuffer(const vk::Buffer bufferSrc, const vk::Buffer bufferDst, const std::vector<vk::BufferCopy>& bufferCopyRegions);

	[[nodiscard]] static vk::CommandBuffer copyBufferToImage(const vk::Buffer buffer, const vk::Image image, const vk::ImageLayout imageLayout, const uint32_t width, const uint32_t height);

private:
	vk_device_memory _deviceMemory;

	vk::Buffer _buffer;

	size_t _size;
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