#include "vk_device_memory.hxx"

#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_device_memory::vk_device_memory()
	: _deviceMemory{}
{
}

vk_device_memory::~vk_device_memory()
{
	free();
}

void vk_device_memory::allocate(const vk::MemoryRequirements& vkMemoryRequirements, const vk::MemoryPropertyFlags vkMemoryPropertyFlags)
{
	vk_renderer* renderer = vk_renderer::get();
	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	const uint32_t memoryTypeIndex =
		findMemoryTypeIndex(physicalDevice.getMemoryProperties(), vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags);

	if (UINT32_MAX != memoryTypeIndex)
	{
		const vk::MemoryAllocateInfo memoryAllocateInfo(vkMemoryRequirements.size, memoryTypeIndex);
		_deviceMemory = device.allocateMemory(memoryAllocateInfo);
	}
}

void vk_device_memory::free()
{
	if (_deviceMemory)
	{
		const vk::Device device = vk_renderer::get()->getDevice();
		device.freeMemory(_deviceMemory);
		_deviceMemory = nullptr;
	}
}

void vk_device_memory::map(const void* data, const vk::DeviceSize size, const vk::DeviceSize offset)
{
	const vk::Device device = vk_renderer::get()->getDevice();
	void* region = device.mapMemory(_deviceMemory, offset, size);
	memcpy(region, data, size);
	device.unmapMemory(_deviceMemory);
}

void vk_device_memory::map(const std::vector<map_memory_region>& regions, const vk::DeviceSize offset)
{
	const vk::Device device = vk_renderer::get()->getDevice();
	void* region = device.mapMemory(_deviceMemory, offset, VK_WHOLE_SIZE);
	for (const auto& reg : regions)
	{
		memcpy(reinterpret_cast<uint8_t*>(region) + reg.offset, reg.data, reg.size);
	}
	device.unmapMemory(_deviceMemory);
}

uint32_t vk_device_memory::findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& vkMemoryProperties, uint32_t memoryTypeBits, vk::MemoryPropertyFlags vkMemoryPropertyFlags)
{
	for (uint32_t i = 0; i < vkMemoryProperties.memoryTypeCount; ++i)
	{
		const bool isRequiredType = (1 << i) & memoryTypeBits;
		const bool hasRequiredProperties =
			(vkMemoryProperties.memoryTypes[i].propertyFlags & vkMemoryPropertyFlags) == vkMemoryPropertyFlags;

		if (isRequiredType && hasRequiredProperties)
		{
			return i;
		}
	}
	return UINT32_MAX;
}