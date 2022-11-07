#include "vk_settings.hxx"

#include "vk_renderer.hxx"

static vk::SampleCountFlagBits findMaxSampleCount(const vk::PhysicalDevice physicalDevice)
{
	const auto limits = physicalDevice.getProperties().limits;
	const auto counts = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
	if (counts & vk::SampleCountFlagBits::e64)
		return vk::SampleCountFlagBits::e64;
	else if (counts & vk::SampleCountFlagBits::e32)
		return vk::SampleCountFlagBits::e32;
	else if (counts & vk::SampleCountFlagBits::e16)
		return vk::SampleCountFlagBits::e16;
	else if (counts & vk::SampleCountFlagBits::e8)
		return vk::SampleCountFlagBits::e8;
	else if (counts & vk::SampleCountFlagBits::e4)
		return vk::SampleCountFlagBits::e4;
	else if (counts & vk::SampleCountFlagBits::e2)
		return vk::SampleCountFlagBits::e2;
	return vk::SampleCountFlagBits::e1;
}

static vk::SurfaceFormatKHR findSurfaceFormat(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

	for (const auto& surfaceFormat : surfaceFormats)
	{
		if (vk::Format::eB8G8R8A8Unorm == surfaceFormat.format)
		{
			return surfaceFormat;
		}
	}

	throw std::runtime_error("Failed to find preffered surface format!");
	return vk::SurfaceFormatKHR();
}

static vk::PresentModeKHR findPresentMode(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
	// clang-format off
	const std::array<vk::PresentModeKHR, 4> priorityModes =
		{
			vk::PresentModeKHR::eMailbox,
			vk::PresentModeKHR::eFifoRelaxed,
			vk::PresentModeKHR::eFifo,
			vk::PresentModeKHR::eImmediate
		};
	// clang-format on

	const auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
	const auto availableModesBegin = availableModes.begin();
	const auto availableModesEnd = availableModes.end();
	for (const auto mode : priorityModes)
	{
		if (std::find(availableModesBegin, availableModesEnd, mode) != availableModesEnd)
		{
			return mode;
		}
	}
	return vk::PresentModeKHR::eImmediate;
}

void vk_settings::init(const vk_renderer* renderer)
{
	const vk::SurfaceKHR surface = renderer->getSurface();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	_surfaceFormat = findSurfaceFormat(physicalDevice, surface);
	_presentMode = findPresentMode(physicalDevice, surface);
	_maxSampleCount = findMaxSampleCount(physicalDevice);

	setPrefferedSampleCount(_maxSampleCount);
}

const vk::SurfaceFormatKHR& vk_settings::getSurfaceFormat() const
{
	return _surfaceFormat;
}

vk::PresentModeKHR vk_settings::getPresentMode() const
{
	return _presentMode;
}

vk::SampleCountFlagBits vk_settings::getMaxSampleCount() const
{
	return _maxSampleCount;
}

vk::SampleCountFlagBits vk_settings::getPrefferedSampleCount() const
{
	return _prefferedSampleCount;
}

bool vk_settings::setPrefferedSampleCount(const vk::SampleCountFlagBits sampleCount)
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

bool vk_settings::getIsSamplingSupported() const
{
	return _prefferedSampleCount != vk::SampleCountFlagBits::e1;
}

vk::PolygonMode vk_settings::getDefaultPolygonMode() const
{
	return _polygonMode;
}

bool vk_settings::setDefaultPolygonMode(const vk::PolygonMode mode)
{
	const bool isDifferent = _polygonMode != mode;
	if (isDifferent)
	{
		_polygonMode = mode;
	}
	return isDifferent;
}
