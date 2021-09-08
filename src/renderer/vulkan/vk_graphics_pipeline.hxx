#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class vk_descriptor_set;

class vk_graphics_pipeline final
{
public:
	vk_graphics_pipeline();
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = delete;
	~vk_graphics_pipeline();

	void create();

	void recreatePipeline();

	void destroy();

	const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const;

	VkPipelineLayout getLayout() const;

	VkPipeline get() const;

protected:
	void createDescriptorLayouts(const VkDevice vkDevice);

	void createPipelineLayout(const VkDevice vkDevice);

	void createPipeline(const VkDevice vkDevice, const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

private:
	std::vector<VkDescriptorSetLayout> _vkDescriptorSetLayouts;

	VkPipelineLayout _vkPipelineLayout;

	VkPipeline _vkPipeline;
};