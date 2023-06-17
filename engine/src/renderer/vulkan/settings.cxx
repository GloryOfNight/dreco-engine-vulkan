#include "settings.hxx"

#include "renderer.hxx"
#include "utils.hxx"

void de::vulkan::settings::init()
{
	const vk::PhysicalDevice physicalDevice = renderer::get()->getPhysicalDevice();
	setSampleCount(utils::findMaxSampleCount(physicalDevice));
}

vk::SampleCountFlagBits de::vulkan::settings::getSampleCount() const
{
	return _sampleCount;
}

bool de::vulkan::settings::IsMultisamplingSupported() const
{
	return _sampleCount != vk::SampleCountFlagBits::e1;
}

vk::PolygonMode de::vulkan::settings::getPolygonMode() const
{
	return _polygonMode;
}

de::vulkan::settings& de::vulkan::settings::setSampleCount(const vk::SampleCountFlagBits sampleCount)
{
	const auto maxSampleCount = utils::findMaxSampleCount(renderer::get()->getPhysicalDevice());
	if (sampleCount <= maxSampleCount)
	{
		_sampleCount = sampleCount;
	}
	return *this;
}

de::vulkan::settings& de::vulkan::settings::setPolygonMode(const vk::PolygonMode mode)
{
	_polygonMode = mode;
	return *this;
}
