#include "vk_physical_device.hxx"

#include "vk_queue_family.hxx"

#include <stdexcept>
#include <vector>

vk_physical_device::vk_physical_device(const VkInstance* vkInstance)
	: _vkInstance{vkInstance}
	, _vkPhysicalDevice{VK_NULL_HANDLE}
	, _vkPhysicalDeviceProperties{}
	, _vkPhysicalDeviceFeatures{}
	, _vkPhysicalDeviceMemoryProperties{}
{
}

void vk_physical_device::setup(VkSurfaceKHR vkSurface)
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(*_vkInstance, &gpuCount, nullptr);
	std::vector<VkPhysicalDevice> gpuList(gpuCount);
	vkEnumeratePhysicalDevices(*_vkInstance, &gpuCount, gpuList.data());

	for (auto& gpu : gpuList)
	{
		if (vk_queue_family(gpu, vkSurface).isVulkanSupported())
		{
			_vkPhysicalDevice = gpu;
			vkGetPhysicalDeviceProperties(gpu, &_vkPhysicalDeviceProperties);
			vkGetPhysicalDeviceFeatures(gpu, &_vkPhysicalDeviceFeatures);
			vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_vkPhysicalDeviceMemoryProperties);
			break;
		}
	}

	if (VK_NULL_HANDLE == _vkPhysicalDevice)
	{
		throw std::runtime_error("No supported GPU found!");
	}
}

VkPhysicalDevice vk_physical_device::get() const
{
	return _vkPhysicalDevice;
}

const VkPhysicalDeviceProperties& vk_physical_device::getProperties() const
{
	return _vkPhysicalDeviceProperties;
}

const VkPhysicalDeviceFeatures& vk_physical_device::getFeatures() const
{
	return _vkPhysicalDeviceFeatures;
}

const VkPhysicalDeviceMemoryProperties& vk_physical_device::getMemoryProperties() const
{
	return _vkPhysicalDeviceMemoryProperties;
}
