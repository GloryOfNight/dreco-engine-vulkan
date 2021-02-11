#include "vk_texture_image.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

vk_texture_image::vk_texture_image()
	: _vkImage{VK_NULL_HANDLE}
	, _vkImageView{VK_NULL_HANDLE}
	, _vkSampler{VK_NULL_HANDLE}
{
}

vk_texture_image::~vk_texture_image()
{
	destroy();
}

void vk_texture_image::create()
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("content/doge.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	const VkFormat vkFormat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;

	createImage(vkDevice, vkFormat, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	vk_buffer_create_info info;
	info.memory_properties_flags = vk_device_memory_properties::HOST;
	info.size = memoryRequirements.size;
	info.usage = vk_buffer_usage::TRANSFER_SRC;
	vk_buffer stagingBuffer;
	stagingBuffer.create(info);

	stagingBuffer.getDeviceMemory().map(pixels, memoryRequirements.size);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);

	transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	vk_buffer::copyBufferToImage(stagingBuffer.get(), _vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	createImageView(vkDevice, vkFormat);
	createSampler(vkDevice);

	stbi_image_free(pixels);
}

void vk_texture_image::destroy()
{
	const VkDevice vkDevice{vk_renderer::get()->getDevice().get()};
	if (VK_NULL_HANDLE != _vkSampler)
	{
		vkDestroySampler(vkDevice, _vkSampler, vkGetAllocator());
		_vkSampler = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != _vkImageView)
	{
		vkDestroyImageView(vkDevice, _vkImageView, vkGetAllocator());
		_vkImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != _vkImage)
	{
		vkDestroyImage(vkDevice, _vkImage, vkGetAllocator());
		_vkImage = VK_NULL_HANDLE;
	}
	_deviceMemory.free();
}

VkImage vk_texture_image::get() const
{
	return _vkImage;
}

VkImageView vk_texture_image::getImageView() const
{
	return _vkImageView;
}

VkSampler vk_texture_image::getSampler() const
{
	return _vkSampler;
}

vk_device_memory& vk_texture_image::getDeviceMemory()
{
	return _deviceMemory;
}

void vk_texture_image::transitionImageLayout(const VkImage vkImage, const VkFormat vkFormat, const VkImageLayout vkLayoutOld, const VkImageLayout vkLayoutNew,
	const VkAccessFlags vkAccessFlagsSrc, const VkAccessFlags vkAccessFlagsDst,
	const VkPipelineStageFlags vkPipelineStageFlagsSrc, const VkPipelineStageFlags vkPipelineStageFlagsDst)
{
	vk_renderer* renderer{vk_renderer::get()};
	VkCommandBuffer vkCommandBuffer = renderer->beginSingleTimeGraphicsCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = vkLayoutOld;
	barrier.newLayout = vkLayoutNew;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vkImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = vkAccessFlagsSrc;
	barrier.dstAccessMask = vkAccessFlagsDst;

	vkCmdPipelineBarrier(vkCommandBuffer, vkPipelineStageFlagsSrc, vkPipelineStageFlagsDst, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	renderer->endSingleTimeGraphicsCommands(vkCommandBuffer);
}

void vk_texture_image::createImage(const VkDevice vkDevice, const VkFormat vkFormat, const uint32_t width, const uint32_t height)
{
	const vk_queue_family& queueFamily{vk_renderer::get()->getQueueFamily()};
	std::vector<uint32_t> queueIndexes;

	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = vkFormat;
	createInfo.extent = VkExtent3D{width, height, 1};
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_LINEAR;
	createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.sharingMode = queueFamily.getSharingMode();
	if (VK_SHARING_MODE_CONCURRENT == createInfo.sharingMode)
	{
		queueIndexes = queueFamily.getUniqueQueueIndexes();
		createInfo.queueFamilyIndexCount = queueIndexes.size();
		createInfo.pQueueFamilyIndices = queueIndexes.data();
	}
	

	VK_CHECK(vkCreateImage(vkDevice, &createInfo, vkGetAllocator(), &_vkImage));
}

void vk_texture_image::bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset)
{
	VK_CHECK(vkBindImageMemory(vkDevice, _vkImage, vkDeviceMemory, memoryOffset));
}

void vk_texture_image::createImageView(const VkDevice vkDevice, const VkFormat vkFormat)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.image = _vkImage;
	createInfo.format = vkFormat;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;

	VK_CHECK(vkCreateImageView(vkDevice, &createInfo, vkGetAllocator(), &_vkImageView));
}

void vk_texture_image::createSampler(const VkDevice vkDevice)
{
	const vk_physical_device& physicalDevice = vk_renderer::get()->getPhysicalDevice();
	const VkPhysicalDeviceProperties& physicalDeviceProperties = physicalDevice.getProperties();
	const VkPhysicalDeviceFeatures& physicalDeviceFeatures = physicalDevice.getFeatures();

	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.mipLodBias = 0.0F;
	createInfo.anisotropyEnable = physicalDeviceFeatures.samplerAnisotropy;
	createInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.minLod = 0.0F;
	createInfo.maxLod = 0.0F;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(vkDevice, &createInfo, vkGetAllocator(), &_vkSampler);
}
