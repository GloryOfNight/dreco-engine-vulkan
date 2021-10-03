#include "vk_settings.hxx"

#include "vk_renderer.hxx"

vk_settings::vk_settings()
	: _maxSampleCount{VK_SAMPLE_COUNT_1_BIT}
	, _prefferedSampleCount{VK_SAMPLE_COUNT_1_BIT}
	, _polygonMode{VK_POLYGON_MODE_FILL}
{
}

void vk_settings::init(const vk_renderer* renderer)
{
	_maxSampleCount = renderer->getPhysicalDevice().getMaxSupportedSampleCount();
	_prefferedSampleCount = _maxSampleCount;
}

VkSampleCountFlagBits vk_settings::getMaxSampleCount() const
{
	return _maxSampleCount;
}

VkSampleCountFlagBits vk_settings::getPrefferedSampleCount() const
{
	return _prefferedSampleCount;
}

bool vk_settings::setPrefferedSampleCount(const VkSampleCountFlagBits sampleCount)
{
	const bool isSupported = sampleCount > 0 &&
							 (sampleCount == VK_SAMPLE_COUNT_1_BIT || sampleCount % 2 == 0) &&
							 _maxSampleCount >= sampleCount;
	if (isSupported)
	{
		_prefferedSampleCount = sampleCount;
	}
	return isSupported;
}

bool vk_settings::getIsSamplingSupported() const
{
	return _prefferedSampleCount != VK_SAMPLE_COUNT_1_BIT;
}

VkPolygonMode vk_settings::getDefaultPolygonMode() const
{
	return _polygonMode;
}

bool vk_settings::setDefaultPolygonMode(const VkPolygonMode mode)
{
	const bool isDifferent = _polygonMode != mode;
	if (isDifferent)
	{
		_polygonMode = mode;
	}
	return isDifferent;
}
