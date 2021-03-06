#include "vk_device.hxx"

#include "vk_allocator.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_utils.hxx"

#include <array>
#include <set>
#include <vector>

vk_device::vk_device()
	: _vkDevice{VK_NULL_HANDLE}
	, _vkGraphicsQueue{VK_NULL_HANDLE}
	, _vkPresentQueue{VK_NULL_HANDLE}
	, _vkTransferQueue{VK_NULL_HANDLE}
{
}

vk_device::~vk_device()
{
	destroy();
}

void vk_device::create(const vk_physical_device& physical_device, const vk_queue_family& queue_family)
{
	const auto& uniqueQueueIndexes = queue_family.getUniqueQueueIndexes();

	const float priorities{1.0F};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	queueCreateInfoList.reserve(uniqueQueueIndexes.size());

	for (const auto& i : uniqueQueueIndexes)
	{
		VkDeviceQueueCreateInfo deviceQueueInfo{};
		deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueInfo.queueFamilyIndex = i;
		deviceQueueInfo.queueCount = 1;
		deviceQueueInfo.pQueuePriorities = &priorities;

		queueCreateInfoList.push_back(deviceQueueInfo);
	}

	const std::array<const char*, 1> deviceExtensions{"VK_KHR_swapchain"};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &physical_device.getFeatures();

	VK_CHECK(vkCreateDevice(physical_device.get(), &deviceCreateInfo, vkGetAllocator(), &_vkDevice));

	vkGetDeviceQueue(_vkDevice, queue_family.getGraphicsIndex(), 0, &_vkGraphicsQueue);
	vkGetDeviceQueue(_vkDevice, queue_family.getPresentIndex(), 0, &_vkPresentQueue);
	vkGetDeviceQueue(_vkDevice, queue_family.getTransferIndex(), 0, &_vkTransferQueue);
}

void vk_device::waitIdle()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		vkDeviceWaitIdle(_vkDevice);
	}
}

void vk_device::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		vkDestroyDevice(_vkDevice, vkGetAllocator());
		_vkDevice = VK_NULL_HANDLE;
	}
}

VkDevice vk_device::get() const
{
	return _vkDevice;
}

VkQueue vk_device::getGraphicsQueue() const
{
	return _vkGraphicsQueue;
}

VkQueue vk_device::getPresentQueue() const
{
	return _vkPresentQueue;
}

VkQueue vk_device::getTransferQueue() const
{
	return _vkTransferQueue;
}
