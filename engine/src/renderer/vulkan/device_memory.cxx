#include "device_memory.hxx"

#include "renderer.hxx"
#include "utils.hxx"

void de::vulkan::device_memory::allocate(const vk::MemoryRequirements& memoryRequirements, const vk::MemoryPropertyFlags memoryPropertyFlags)
{
	renderer* renderer = renderer::get();
	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	const uint32_t memoryTypeIndex =
		findMemoryTypeIndex(physicalDevice.getMemoryProperties(), memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	if (UINT32_MAX != memoryTypeIndex)
	{
		const vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
		_deviceMemory = device.allocateMemory(memoryAllocateInfo);
	}
}

void de::vulkan::device_memory::free()
{
	if (_deviceMemory)
	{
		const vk::Device device = renderer::get()->getDevice();
		device.freeMemory(_deviceMemory);
		_deviceMemory = nullptr;
	}
}

void de::vulkan::device_memory::map(const void* data, const vk::DeviceSize size, const vk::DeviceSize offset)
{
	const vk::Device device = renderer::get()->getDevice();
	void* region = device.mapMemory(_deviceMemory, offset, size);
	memcpy(region, data, size);
	device.unmapMemory(_deviceMemory);
}

void de::vulkan::device_memory::map(const std::vector<map_memory_region>& regions, const vk::DeviceSize offset)
{
	if (regions.empty())
	{
		return;
	}

	const vk::Device device = renderer::get()->getDevice();
	void* region = device.mapMemory(_deviceMemory, offset, VK_WHOLE_SIZE);
	for (const auto& reg : regions)
	{
		memcpy(reinterpret_cast<uint8_t*>(region) + reg.offset, reg.data, reg.size);
	}
	device.unmapMemory(_deviceMemory);
}

uint32_t de::vulkan::device_memory::findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const bool isRequiredType = (1 << i) & memoryTypeBits;
		const bool hasRequiredProperties =
			(memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags;

		if (isRequiredType && hasRequiredProperties)
		{
			return i;
		}
	}
	return UINT32_MAX;
}