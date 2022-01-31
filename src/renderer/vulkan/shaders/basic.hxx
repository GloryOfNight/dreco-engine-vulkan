#pragma once
#include "renderer/vulkan/vk_shader.hxx"

class vk_shader_basic : public vk_shader
{
public:
	vk_shader_basic();

	std::vector<vk::PipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos() const override;

	std::vector<vk::DescriptorSetLayoutBinding> getDescriptorSetLayoutBindings() const override;
};