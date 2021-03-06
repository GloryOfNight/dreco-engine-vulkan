#include "vk_physical_device.hxx"

#include <stdexcept>
#include <vector>

vk_physical_device::vk_physical_device()
	: _vkPhysicalDevice{VK_NULL_HANDLE}
	, _vkPhysicalDeviceProperties{}
	, _vkPhysicalDeviceFeatures{}
	, _vkPhysicalDeviceMemoryProperties{}
{
}

void vk_physical_device::setup(const VkInstance vkInstance, VkSurfaceKHR vkSurface)
{
	
	uint32_t gpuCount{0};
	vkEnumeratePhysicalDevices(vkInstance, &gpuCount, nullptr);
	std::vector<VkPhysicalDevice> gpuList(gpuCount);
	vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpuList.data());

	// clang-format off
	auto isGpuSuitSurface = [vkSurface](const VkPhysicalDevice vkPhysicalDevice) -> bool 
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			VkBool32 isQueueFamilySupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &isQueueFamilySupported);
			if (isQueueFamilySupported)
			{
				return true;
			}
		}
		return false;
	};
	// clang-format on

	for (auto& gpu : gpuList)
	{
		if (isGpuSuitSurface(gpu))
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