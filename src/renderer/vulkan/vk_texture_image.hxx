#pragma once

#include "vk_device_memory.hxx"

#include <vulkan/vulkan.h>

class vk_texture_image final
{
public:
	vk_texture_image();
	vk_texture_image(const vk_texture_image&) = delete;
	vk_texture_image(vk_texture_image&&) = delete;
	~vk_texture_image();

	void create();

	void destroy();

	VkImage get() const;

	VkImageView getImageView() const;

	VkSampler getSampler() const;

	vk_device_memory& getDeviceMemory();

	static void transitionImageLayout(const VkImage vkImage, const VkFormat vkFormat, const VkImageLayout vkLayoutOld, const VkImageLayout vkLayoutNew,
		const VkAccessFlags vkAccessFlagsSrc, const VkAccessFlags vkAccessFlagsDst, 
		const VkPipelineStageFlags vkPipelineStageFlagsSrc, const VkPipelineStageFlags vkPipelineStageFlagsDst);

protected:
	void createImage(const VkDevice vkDevice, const VkFormat vkFormat, const uint32_t width, const uint32_t height);

	void bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset);

	void createImageView(const VkDevice vkDevice, const VkFormat vkFormat);

	void createSampler(const VkDevice vkDevice);

private:
	VkImage _vkImage;

	vk_device_memory _deviceMemory;

	VkImageView _vkImageView;

	VkSampler _vkSampler;
};