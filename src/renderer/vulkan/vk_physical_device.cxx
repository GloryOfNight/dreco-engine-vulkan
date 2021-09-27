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
			vkGetPhysicalDeviceProperties(gpu, &_vkPhysicalDeviceProperties);
			if (VK_PHYSICAL_DEVICE_TYPE_CPU == _vkPhysicalDeviceProperties.deviceType ||
				VK_PHYSICAL_DEVICE_TYPE_OTHER == _vkPhysicalDeviceProperties.deviceType)
			{
				continue;
			}

			_vkPhysicalDevice = gpu;
			vkGetPhysicalDeviceFeatures(gpu, &_vkPhysicalDeviceFeatures);
			vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_vkPhysicalDeviceMemoryProperties);
			if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == _vkPhysicalDeviceProperties.deviceType)
			{
				break;
			}
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

VkFormat vk_physical_device::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features) const
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	return VkFormat::VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits vk_physical_device::getMaxSupportedSampleCount() const
{
	const auto& limits = _vkPhysicalDeviceProperties.limits;
	const auto counts = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}