#include "vk_texture_image.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"

vk_texture_image::vk_texture_image()
	: _vkImage{VK_NULL_HANDLE}
{
}

vk_texture_image::~vk_texture_image()
{
	destroy();
}

void vk_texture_image::create()
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	createImage(vkDevice);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);
}

void vk_texture_image::destroy()
{
	if (VK_NULL_HANDLE != _vkImage)
	{
		vkDestroyImage(vk_renderer::get()->getDevice().get(), _vkImage, vkGetAllocator());
		_vkImage = VK_NULL_HANDLE;
	}
	_deviceMemory.destroy();
}

VkImage vk_texture_image::get() const
{
	return _vkImage;
}

vk_device_memory& vk_texture_image::getDeviceMemory()
{
	return _deviceMemory;
}

void vk_texture_image::createImage(const VkDevice vkDevice)
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkSharingMode sharingMode = renderer->getQueueFamily().getSharingMode();

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
	imageCreateInfo.extent = VkExtent3D{512, 512, 0};
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR;
	imageCreateInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = sharingMode;
	if (sharingMode == VkSharingMode::VK_SHARING_MODE_CONCURRENT)
	{
		const std::vector<uint32_t> queueIndexes{renderer->getQueueFamily().getQueueIndexes()};
		imageCreateInfo.queueFamilyIndexCount = queueIndexes.size();
		imageCreateInfo.pQueueFamilyIndices = queueIndexes.data();
	}
	imageCreateInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(vkDevice, &imageCreateInfo, vkGetAllocator(), &_vkImage);
}

void vk_texture_image::bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset)
{
	bindToMemory(vkDevice, vkDeviceMemory, memoryOffset);
}
