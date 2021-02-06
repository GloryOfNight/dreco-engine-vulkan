#pragma once

#include "vk_device_memory.hxx"

#include <vulkan/vulkan.h>

class vk_texture_image
{
public:
	vk_texture_image();
	vk_texture_image(const vk_texture_image&) = delete;
	vk_texture_image(vk_texture_image&&) = delete;
	~vk_texture_image();

	void create();

	void destroy();

	VkImage get() const;

	vk_device_memory& getDeviceMemory();

protected:

	void createImage(const VkDevice vkDevice);

	void bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset);

private:
	VkImage _vkImage;
	
	vk_device_memory _deviceMemory;
};