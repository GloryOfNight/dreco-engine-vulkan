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
	vk_descriptor_set(vk_descriptor_set&&) = default;
	~vk_descriptor_set();

	void create(const std::vector<vk_graphics_pipeline*>& pipelines, const std::vector<vk_texture_image*>& textureImages);

	void rewrite(const std::pair<uint32_t, vk_texture_image*>& _textureImage);

	void update(const std::vector<vk::WriteDescriptorSet>& writeInfo);

	void bindToCmdBuffer(vk::CommandBuffer commandBuffer);

	void destroy();

	const std::vector<vk::DescriptorSet>& get() const { return _descriptorSets; };

	vk_buffer& getUniformBuffer();

protected:
	void createDescriptorPool(const vk::Device device, const size_t count);

	vk::DescriptorSet createDescriptorSet(const vk::Device device, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);

	// ex's required because we cannot allocate them on stask inside this function
	void createWriteForDescriptorSet(uint32_t index, std::vector<vk::WriteDescriptorSet>& outWrite,
		vk::DescriptorBufferInfo& exBufferInfo, vk::DescriptorImageInfo& exImageInfo, const std::vector<vk_texture_image*>& textureImages);

	void createWriteForDescriptorSet(uint32_t index, std::vector<vk::WriteDescriptorSet>& outWrite,
		vk::DescriptorBufferInfo& exBufferInfo, vk::DescriptorImageInfo& exImageInfo, const vk_texture_image* texImage);

	void createUniformBuffer();

private:
	vk_buffer _uniformBuffer;

	vk::DescriptorPool _descriptorPool;

	std::vector<vk::DescriptorSet> _descriptorSets;

	std::vector<vk_graphics_pipeline*> _pipelines;
};