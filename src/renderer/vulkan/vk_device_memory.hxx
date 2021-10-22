#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_device_memory final
{
public:
	struct map_memory_region
	{
		const void* data{nullptr};
		const vk::DeviceSize size{0};
		const vk::DeviceSize offset{0};
	};

	vk_device_memory();
	vk_device_memory(const vk_device_memory&) = delete;
	vk_device_memory(vk_device_memory&&) = default;
	~vk_device_memory();

	void allocate(const vk::MemoryRequirements& memoryRequirements, const vk::MemoryPropertyFlags memoryPropertyFlags);

	void free();

	void map(const void* data, const vk::DeviceSize size, const vk::DeviceSize offset = 0);

	void map(const std::vector<map_memory_region>& regions, const vk::DeviceSize offset = 0);

	vk::DeviceMemory get() const;

protected:
	static uint32_t findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
		uint32_t memoryTypeBits, vk::MemoryPropertyFlags memoryPropertyFlags);

private:
	vk::DeviceMemory _deviceMemory;
};