#pragma once
#include "renderer/vulkan/vk_shader.hxx"

class vk_shader_basic_vert : public vk_shader
{
public:
	vk_shader_basic_vert();

	void addPipelineShaderStageCreateInfo(std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages) const override;

	void addDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings) const override;

	void addPushConstantRange(std::vector<vk::PushConstantRange>& ranges) const override;

	void addDescriptorPoolSizes(std::vector<vk::DescriptorPoolSize>& sizes) const override;

	void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const override;

	void cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh);
};

class vk_shader_basic_frag : public vk_shader
{
public:
	vk_shader_basic_frag();

	void addPipelineShaderStageCreateInfo(std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages) const override;

	void addDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings) const override;

	void addDescriptorPoolSizes(std::vector<vk::DescriptorPoolSize>& sizes) const override;

	void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const override;
};