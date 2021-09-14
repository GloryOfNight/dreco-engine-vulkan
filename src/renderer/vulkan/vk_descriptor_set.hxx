#pragma once
#include "vk_buffer.hxx"

#include <vector>
#include <vulkan/vulkan.h>

class vk_graphics_pipeline;
class vk_texture_image;

class vk_descriptor_set final
{
public:
	vk_descriptor_set();
	vk_descriptor_set(const vk_descriptor_set&) = delete;
	vk_descriptor_set(vk_descriptor_set&&) = delete;
	~vk_descriptor_set();

	void create(const std::vector<vk_graphics_pipeline*>& pipelines, const std::vector<vk_texture_image*>& textureImages);

	void rewrite(const std::pair<uint32_t, vk_texture_image*>& _textureImage);

	void update(const std::vector<VkWriteDescriptorSet>& writeInfo);

	void bindToCmdBuffer(VkCommandBuffer commandBuffer);

	void destroy();

	const std::vector<VkDescriptorSet>& get() const;

	vk_buffer& getUniformBuffer();

protected:
	void createDescriptorPool(const VkDevice vkDevice, const size_t count);

	VkDescriptorSet createDescriptorSet(const VkDevice vkDevice, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

	// ex's required because we cannot allocate them on stask inside this function
	void createWriteForDescriptorSet(uint32_t index, std::vector<VkWriteDescriptorSet>& outWrite,
		VkDescriptorBufferInfo& exBufferInfo, VkDescriptorImageInfo& exImageInfo, const std::vector<vk_texture_image*>& textureImages);

	void createWriteForDescriptorSet(uint32_t index, std::vector<VkWriteDescriptorSet>& outWrite,
		VkDescriptorBufferInfo& exBufferInfo, VkDescriptorImageInfo& exImageInfo, const vk_texture_image* texImage);

	void createUniformBuffer();

private:
	vk_buffer _uniformBuffer;

	VkDescriptorPool _vkDescriptorPool;

	std::vector<VkDescriptorSet> _vkDescriptorSets;

	std::vector<vk_graphics_pipeline*> _pipelines;
};