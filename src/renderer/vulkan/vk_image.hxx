#pragma once
#include "vk_device_memory.hxx"

#include <vulkan/vulkan.h>

class vk_image
{
public:
    virtual ~vk_image() = default;
	virtual void create() = 0;

    virtual void destroy();

    VkImage getImage() const {return _vkImage;};

    VkImageView getImageView() const {return _vkImageView;};

    const vk_device_memory& getDeviceMemory() const {return _deviceMemory;};

    static void transitionImageLayout(const VkImage vkImage, const VkFormat vkFormat, const VkImageLayout vkLayoutOld, const VkImageLayout vkLayoutNew,
		const VkAccessFlags vkAccessFlagsSrc, const VkAccessFlags vkAccessFlagsDst,
		const VkPipelineStageFlags vkPipelineStageFlagsSrc, const VkPipelineStageFlags vkPipelineStageFlagsDst, const VkImageAspectFlags vkAspectFlags);

protected:
    virtual VkImageAspectFlags getImageAspectFlags() const;

	virtual VkImageUsageFlags getImageUsageFlags() const;

    void createImage(const VkDevice vkDevice, const VkFormat vkFormat, const uint32_t width, const uint32_t height, const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

	void bindToMemory(const VkDevice vkDevice, const VkDeviceMemory vkDeviceMemory, const VkDeviceSize memoryOffset);

	void createImageView(const VkDevice vkDevice, const VkFormat vkFormat);

    VkImage _vkImage{VK_NULL_HANDLE};

    VkImageView _vkImageView{VK_NULL_HANDLE};

    vk_device_memory _deviceMemory;
};