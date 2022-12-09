#pragma once
#include <vulkan/vulkan.hpp>

namespace de::vulkan
{
	class device_memory final
	{
	public:
		struct map_memory_region
		{
			const void* data{nullptr};
			const vk::DeviceSize size{0};
			const vk::DeviceSize offset{0};
		};

		device_memory() = default;
		device_memory(const device_memory&) = delete;
		device_memory(device_memory&&) = default;
		~device_memory() { free(); };

		void allocate(const vk::MemoryRequirements& memoryRequirements, const vk::MemoryPropertyFlags memoryPropertyFlags);

		void free();

		void map(const void* data, const vk::DeviceSize size, const vk::DeviceSize offset = 0);

		void map(const std::vector<map_memory_region>& regions, const vk::DeviceSize offset = 0);

		vk::DeviceMemory get() const { return _deviceMemory; };

	protected:
		static uint32_t findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
			uint32_t memoryTypeBits, vk::MemoryPropertyFlags memoryPropertyFlags);

	private:
		vk::DeviceMemory _deviceMemory;
	};
} // namespace de::vulkan