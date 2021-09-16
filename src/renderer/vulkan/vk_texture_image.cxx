#include "vk_texture_image.hxx"

#include "core/utils/log.hxx"

#include "vk_allocator.hxx"
#include "vk_renderer.hxx"
#include "vk_utils.hxx"

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
	auto* texture = texture_data::createNew(TEXTURE_PLACEHOLDER_URI);
	unsigned char* pixels{nullptr};
	texture->getData(&pixels, nullptr, nullptr, nullptr);
	if (nullptr == pixels)
	{
		DR_LOGF(Critical, "Failed to load placeholder texture! Cannot proceed. . .");
		std::abort();
		return;
	}
	create(*texture);
	delete texture;
}

void vk_texture_image::create(const image& img)
{
	auto* texture = texture_data::createNew(img._uri);
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
		DR_LOGF(Error, "No valid texture data, trying use placeholder instead: %s", TEXTURE_PLACEHOLDER_URI);
		create();
		return;
	}

	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice{renderer->getDevice().get()};

	const VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

	createImage(vkDevice, vkFormat, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(vkDevice, _vkImage, &memoryRequirements);

	_deviceMemory.allocate(memoryRequirements, static_cast<VkMemoryPropertyFlags>(vk_device_memory_properties::DEVICE_ONLY));

	bindToMemory(vkDevice, _deviceMemory.get(), 0);

	createImageView(vkDevice, vkFormat);
	createSampler(vkDevice);

	vk_buffer_create_info info;
	info.memory_properties_flags = vk_device_memory_properties::HOST;
	info.size = memoryRequirements.size;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	vk_buffer stagingBuffer;
	stagingBuffer.create(info);
	stagingBuffer.getDeviceMemory().map(pixels, texWidth * texHeight * 4);

	std::array<VkSemaphore, 2> semaphores;
	VkSemaphoreCreateInfo semaphoreCreareInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
	vkCreateSemaphore(vkDevice, &semaphoreCreareInfo, vkGetAllocator(), &semaphores[0]);
	vkCreateSemaphore(vkDevice, &semaphoreCreareInfo, vkGetAllocator(), &semaphores[1]);

	std::array<VkCommandBuffer, 3> commandBuffers;

	commandBuffers[0] = transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, getImageAspectFlags());

	commandBuffers[1] = vk_buffer::copyBufferToImage(stagingBuffer.get(), _vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	commandBuffers[2] = transitionImageLayout(_vkImage, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, getImageAspectFlags());

	std::array<VkPipelineStageFlags, 2> stageFlags{
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
	std::vector<VkSubmitInfo> submitInfos(3, VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO});
	submitInfos[0].commandBufferCount = 1;
	submitInfos[0].pCommandBuffers = &commandBuffers[0];
	submitInfos[0].waitSemaphoreCount = 0;
	submitInfos[0].pWaitSemaphores = VK_NULL_HANDLE;
	submitInfos[0].signalSemaphoreCount = 1;
	submitInfos[0].pSignalSemaphores = &semaphores[0];
	submitInfos[0].pWaitDstStageMask = &stageFlags[0];

	submitInfos[1].commandBufferCount = 1;
	submitInfos[1].pCommandBuffers = &commandBuffers[1];
	submitInfos[1].waitSemaphoreCount = 1;
	submitInfos[1].pWaitSemaphores = semaphores.data();
	submitInfos[1].signalSemaphoreCount = 1;
	submitInfos[1].pSignalSemaphores = &semaphores[1];
	submitInfos[1].pWaitDstStageMask = &stageFlags[1];

	submitInfos[2].commandBufferCount = 1;
	submitInfos[2].pCommandBuffers = &commandBuffers[2];
	submitInfos[2].waitSemaphoreCount = 1;
	submitInfos[2].pWaitSemaphores = &semaphores[1];
	submitInfos[2].signalSemaphoreCount = 0;
	submitInfos[2].pSignalSemaphores = VK_NULL_HANDLE;
	submitInfos[2].pWaitDstStageMask = &stageFlags[1];

	renderer->submitSingleTimeTransferCommands(submitInfos);

	vkDestroySemaphore(vkDevice, semaphores[0], vkGetAllocator());
	vkDestroySemaphore(vkDevice, semaphores[1], vkGetAllocator());

	vkFreeCommandBuffers(vkDevice, renderer->getTransferCommandPool(), commandBuffers.size(), commandBuffers.data());
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

bool vk_texture_image::isValid() const
{
	return getImage() && getImageView() && getSampler();
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
