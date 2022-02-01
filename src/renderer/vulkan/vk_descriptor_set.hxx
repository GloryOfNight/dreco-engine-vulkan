#pragma once
#include "vk_buffer.hxx"

#include <vector>
#include <vulkan/vulkan.h>

class vk_graphics_pipeline;
class vk_texture_image;

struct vk_descriptor_set_write
{
	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	std::vector<vk::DescriptorImageInfo> imageInfos;
	std::vector<vk::WriteDescriptorSet> descriptorSets;
};

class vk_descriptor_set final
{
public:
	vk_descriptor_set() = default;
	vk_descriptor_set(const vk_descriptor_set&) = delete;
	vk_descriptor_set(vk_descriptor_set&&) = default;
	~vk_descriptor_set() { destroy(); };

	void create(const std::vector<vk_graphics_pipeline*>& pipelines, const std::vector<vk_texture_image*>& textureImages);

	void updateTextureImages(const std::vector<vk_texture_image*>& textureImages);

	void update(const std::vector<vk::WriteDescriptorSet>& writeInfo);

	void bindToCmdBuffer(vk::CommandBuffer commandBuffer);

	void destroy();

	const std::vector<vk::DescriptorSet>& get() const { return _descriptorSets; };

	const std::vector<vk_graphics_pipeline*>& getPipelines() const { return _pipelines; };

protected:
	void createDescriptorPool(const vk::Device device, const size_t count);

	std::vector<vk::DescriptorSet> createDescriptorSets(const vk::Device device, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);

	vk::DescriptorBufferInfo& addWriteBufferInfo(vk_descriptor_set_write& write, const vk_buffer& buffer);

	vk::DescriptorImageInfo& addWriteImageInfo(vk_descriptor_set_write& write, const vk_texture_image& image);

	void writeDescriptorSetsBufferInfos(uint32_t index, vk_descriptor_set_write& write);

	void writeDescriptorSetsImageInfos(uint32_t index, vk_descriptor_set_write& write);

private:
	vk::DescriptorPool _descriptorPool;

	std::vector<vk::DescriptorSet> _descriptorSets;

	std::vector<vk_graphics_pipeline*> _pipelines;
};