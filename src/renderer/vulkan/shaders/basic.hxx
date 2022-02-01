#pragma once
#include "renderer/vulkan/vk_shader.hxx"

class vk_shader_basic_vert : public vk_shader
{
public:
	vk_shader_basic_vert();

	vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const override;

	vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() const override;

	void addPushConstantRange(std::vector<vk::PushConstantRange>& ranges) const override;
};

class vk_shader_basic_frag : public vk_shader
{
public:
	vk_shader_basic_frag();

	vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const override;

	vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() const override;
};