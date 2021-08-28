#include "vk_device_memory.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_device_memory::vk_device_memory()
	: _vkDeviceMemory{VK_NULL_HANDLE}
{
}

vk_device_memory::~vk_device_memory()
{
	free();
}

void vk_device_memory::allocate(const VkMemoryRequirements& vkMemoryRequirements, const VkMemoryPropertyFlags vkMemoryPropertyFlags)
{
	vk_renderer* renderer{vk_renderer::get()};

	// clang-format off
	const uint32_t memoryTypeIndex
	{
		findMemoryTypeIndex(renderer->getPhysicalDevice().getMemoryProperties(),
		vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags)
	};
	// clang-format on

	if (UINT32_MAX != memoryTypeIndex)
	{
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		VK_CHECK(vkAllocateMemory(renderer->getDevice().get(), &memoryAllocateInfo, vkGetAllocator(), &_vkDeviceMemory));
	}
}

void vk_device_memory::free()
{
	if (VK_NULL_HANDLE != _vkDeviceMemory)
	{
		vkFreeMemory(vk_renderer::get()->getDevice().get(), _vkDeviceMemory, vkGetAllocator());
		_vkDeviceMemory = VK_NULL_HANDLE;
	}
}

void vk_device_memory::map(const void* data, const VkDeviceSize size, const VkDeviceSize offset)
{
	const VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	void* region;
	VK_CHECK(vkMapMemory(vkDevice, _vkDeviceMemory, offset, VK_WHOLE_SIZE, 0, &region));
	memcpy(region, data, size);
	vkUnmapMemory(vkDevice, _vkDeviceMemory);
}

VkDeviceMemory vk_device_memory::get() const
{
	return _vkDeviceMemory;
}

uint32_t vk_device_memory::findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& vkMemoryProperties,
	uint32_t memoryTypeBits, VkMemoryPropertyFlags vkMemoryPropertyFlags)
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