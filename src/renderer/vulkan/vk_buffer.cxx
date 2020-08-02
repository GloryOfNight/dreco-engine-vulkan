#include "vk_buffer.hxx"
#include "vk_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_physical_device.hxx"
#include "vk_utils.hxx"
#include <cstring>

vk_buffer::vk_buffer()
	: _device{nullptr}
	, _vkBuffer{VK_NULL_HANDLE}
	, _vkDeviceMemory{VK_NULL_HANDLE}
{
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
			vkDestroyBuffer(vkDevice, vkBuffer, VK_NULL_HANDLE);
			vkBuffer = VK_NULL_HANDLE;
		}
		if (VK_NULL_HANDLE != vkDeviceMemery)
		{
			vkFreeMemory(vkDevice, vkDeviceMemery, VK_NULL_HANDLE);
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
	bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(create_info.usage);
	bufferCreateInfo.sharingMode = create_info.queue_family->getSharingMode();
	if (VK_SHARING_MODE_CONCURRENT == bufferCreateInfo.sharingMode)
	{
		uint32_t queueFamilyIndexes[3] =
		{
			create_info.queue_family->getGraphicsIndex(), 
			create_info.queue_family->getPresentIndex(),
			create_info.queue_family->getTransferIndex()
		};

		bufferCreateInfo.queueFamilyIndexCount = 3;
		bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndexes;
	}

	VK_CHECK(vkCreateBuffer(vkDevice, &bufferCreateInfo, VK_NULL_HANDLE, &vkBuffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memoryRequirements);

	const int32_t memoryTypeIndex = findMemoryTypeIndex
	(
		create_info.physical_device->getMemoryProperties(),
		memoryRequirements.memoryTypeBits,
		static_cast<VkMemoryPropertyFlags>(create_info.memory_properties)
	);

	if (-1 != memoryTypeIndex)
	{
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		VK_CHECK(vkAllocateMemory(vkDevice, &memoryAllocateInfo, VK_NULL_HANDLE, &vkDeviceMemory));

		vkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, 0);
	}
	else
	{
		vkDestroyBuffer(vkDevice, vkBuffer, VK_NULL_HANDLE);
		throw std::runtime_error("No suitable memory find for buffer!");
	}
}

int32_t vk_buffer::findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& vkMemoryProperties,
	uint32_t memoryTypeBits, VkMemoryPropertyFlags vkMemoryPropertyFlags)
{
	for (uint32_t i = 0; i < vkMemoryProperties.memoryTypeCount; ++i)
	{
		const bool isRequeredType = (1 << i) & memoryTypeBits;
		const bool hasRequeredProperties = 
			(vkMemoryProperties.memoryTypes[i].propertyFlags & vkMemoryPropertyFlags) == vkMemoryPropertyFlags;

		if (isRequeredType && hasRequeredProperties)
		{
			return static_cast<int32_t>(i);
		}
	}
	return -1;
}