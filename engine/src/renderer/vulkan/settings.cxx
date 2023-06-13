#include "settings.hxx"

#include "renderer.hxx"
#include "utils.hxx"

void de::vulkan::settings::init(const renderer* renderer)
{
	const vk::SurfaceKHR surface = renderer->getSurface();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	_surfaceFormat = utils::findSurfaceFormat(physicalDevice, surface);
	_presentMode = utils::findPresentMode(physicalDevice, surface);
	_maxSampleCount = utils::findMaxSampleCount(physicalDevice);

	setPrefferedSampleCount(_maxSampleCount);
}

const vk::SurfaceFormatKHR& de::vulkan::settings::getSurfaceFormat() const
{
	return _surfaceFormat;
}

vk::PresentModeKHR de::vulkan::settings::getPresentMode() const
{
	return _presentMode;
}

vk::SampleCountFlagBits de::vulkan::settings::getMaxSampleCount() const
{
	return _maxSampleCount;
}

vk::SampleCountFlagBits de::vulkan::settings::getPrefferedSampleCount() const
{
	return _prefferedSampleCount;
}

bool de::vulkan::settings::setPrefferedSampleCount(const vk::SampleCountFlagBits sampleCount)
{
	const bool isSupported = static_cast<uint32_t>(sampleCount) > 0 &&
							 (sampleCount == vk::SampleCountFlagBits::e1 || static_cast<uint32_t>(sampleCount) % 2 == 0) &&
							 _maxSampleCount >= sampleCount;
	if (isSupported)
	{
		_prefferedSampleCount = sampleCount;
	}
	return isSupported;
}

bool de::vulkan::settings::getIsSamplingSupported() const
{
	return _prefferedSampleCount != vk::SampleCountFlagBits::e1;
}

vk::PolygonMode de::vulkan::settings::getDefaultPolygonMode() const
{
	return _polygonMode;
}

bool de::vulkan::settings::setDefaultPolygonMode(const vk::PolygonMode mode)
{
	const bool isDifferent = _polygonMode != mode;
	if (isDifferent)
	{
		_polygonMode = mode;
	}
	return isDifferent;
}
