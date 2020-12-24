#include "vk_buffer.hxx"

#include "vk_allocator.hxx"
#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_utils.hxx"

#include <cstring>

vk_buffer::vk_buffer()
	: _device{nullptr}
	, _vkBuffer{VK_NULL_HANDLE}
	, _vkDeviceMemory{VK_NULL_HANDLE}
{
}

vk_buffer::vk_buffer(vk_buffer&& other) noexcept
	: _device{other._device}
	, _vkBuffer{other._vkBuffer}
	, _vkDeviceMemory{other._vkDeviceMemory}
{
	other._device = nullptr;
	other._vkBuffer = VK_NULL_HANDLE;
	other._vkDeviceMemory = VK_NULL_HANDLE;
}

vk_buffer::~vk_buffer()
{
	destroy();
}

void vk_buffer::create(const vk_device* device, const vk_buffer_create_info& create_info)
{
	_device = device;
	createBuffer(create_info, _vkBuffer, _device->get(), _vkDeviceMemory);
}

void vk_buffer::destroy()
{
	if (_device)
	{
		destroy(_device->get(), _vkBuffer, _vkDeviceMemory);
	}
}

void vk_buffer::map(const void* data, const VkDeviceSize size)
{
	void* region;
	VK_CHECK(vkMapMemory(_device->get(), _vkDeviceMemory, 0, VK_WHOLE_SIZE, 0, &region));
	memcpy(region, data, size);
	vkUnmapMemory(_device->get(), _vkDeviceMemory);
}

VkBuffer vk_buffer::get() const
{
	return _vkBuffer;
}

void vk_buffer::destroy(VkDevice vkDevice, VkBuffer& vkBuffer, VkDeviceMemory& vkDeviceMemery)
{
	if (VK_NULL_HANDLE != vkDevice)
	{
		if (VK_NULL_HANDLE != vkBuffer)
		{
			vkDestroyBuffer(vkDevice, vkBuffer, vkGetAllocator());
			vkBuffer = VK_NULL_HANDLE;
		}
		if (VK_NULL_HANDLE != vkDeviceMemery)
		{
			vkFreeMemory(vkDevice, vkDeviceMemery, vkGetAllocator());
			vkDeviceMemery = VK_NULL_HANDLE;
		}
		vkDevice = VK_NULL_HANDLE;
	}
}

void vk_buffer::createBuffer(
	const vk_buffer_create_info& create_info, VkBuffer& vkBuffer, VkDevice vkDevice, VkDeviceMemory& vkDeviceMemory)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = create_info.size;
	bufferCreateInfo.usage = static_cast<VkBufferUsageFlagBits>(create_info.usage);
	bufferCreateInfo.sharingMode = create_info.queueFamily->getSharingMode();
	if (VK_SHARING_MODE_CONCURRENT == bufferCreateInfo.sharingMode)
	{
		const auto& uniqueQueueIndexes = create_info.queueFamily->getUniqueQueueIndexes();
		bufferCreateInfo.queueFamilyIndexCount = uniqueQueueIndexes.size();
		bufferCreateInfo.pQueueFamilyIndices = uniqueQueueIndexes.data();
	}

	VK_CHECK(vkCreateBuffer(vkDevice, &bufferCreateInfo, vkGetAllocator(), &vkBuffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memoryRequirements);

	const uint32_t memoryTypeIndex{findMemoryTypeIndex(
		create_info.physicalDevice->getMemoryProperties(),
		memoryRequirements.memoryTypeBits,
		static_cast<VkMemoryPropertyFlags>(create_info.memory_properties))};

	if (UINT32_MAX != memoryTypeIndex)
	{
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		VK_CHECK(vkAllocateMemory(vkDevice, &memoryAllocateInfo, vkGetAllocator(), &vkDeviceMemory));

		vkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, 0);
	}
	else
	{
		vkDestroyBuffer(vkDevice, vkBuffer, VK_NULL_HANDLE);
		throw std::runtime_error("No suitable memory find for buffer!");
	}
}

uint32_t vk_buffer::findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& vkMemoryProperties,
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