#include "vk_texture_image.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

#define VK_TEXTURE_PLACEHOLDER_URI "content/doge.jpg"

vk_texture_image::vk_texture_image()
	: _vkSampler{VK_NULL_HANDLE}
{
}

vk_texture_image::~vk_texture_image()
{
	destroy();
}

void vk_texture_image::create()
{
	auto* texture = texture_data::createNew(VK_TEXTURE_PLACEHOLDER_URI);
	create(*texture);
	delete texture;
}

void vk_texture_image::create(const texture_data& textureData)
{
	int texWidth, texHeight, texChannels;
	unsigned char* pixels{nullptr};
	textureData.getData(&pixels, &texWidth, &texHeight, &texChannels);

	if (!pixels)
	{
		std::cerr << "vk_texture_image: Failed to load texture data, using placeholder texture: " << VK_TEXTURE_PLACEHOLDER_URI << ";\n";
		create();
		return;
	}

	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	const VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

	createImage(vkDevice, vkFormat, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	vk_buffer_create_info info;
	info.memory_properties_flags = vk_device_memory_properties::HOST;
	info.size = memoryRequirements.size;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vk_buffer stagingBuffer;
	stagingBuffer.create(info);

	stagingBuffer.getDeviceMemory().map(pixels, texWidth * texHeight * 4);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);

	transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, getImageAspectFlags());

	vk_buffer::copyBufferToImage(stagingBuffer.get(), _vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, getImageAspectFlags());

	createImageView(vkDevice, vkFormat);
	createSampler(vkDevice);
}

void vk_texture_image::destroy()
{
	const VkDevice vkDevice{vk_renderer::get()->getDevice().get()};
	if (VK_NULL_HANDLE != _vkSampler)
	{
		vkDestroySampler(vkDevice, _vkSampler, vkGetAllocator());
		_vkSampler = VK_NULL_HANDLE;
	}
	vk_image::destroy();
}

VkSampler vk_texture_image::getSampler() const
{
	return _vkSampler;
}

VkImageAspectFlags vk_texture_image::getImageAspectFlags() const
{
	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageUsageFlags vk_texture_image::getImageUsageFlags() const
{
	return VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
