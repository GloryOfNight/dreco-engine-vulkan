#pragma once

#include <vulkan/vulkan.h>

class vk_descriptor_set;

class vk_graphics_pipeline
{
public:
	vk_graphics_pipeline();
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = delete;
	~vk_graphics_pipeline();

	void create(const vk_descriptor_set& vkDescriptorSet);

	void recreatePipeline();

	void destroy();

	VkPipelineLayout getLayout() const;

	VkPipeline get() const;

protected:
	void createPipelineLayout(const VkDevice vkDevice, const vk_descriptor_set& vkDescriptorSet);

	void createPipeline(const VkDevice vkDevice, const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

private:
	VkPipelineLayout _vkPipelineLayout;

	VkPipeline _vkPipeline;
};