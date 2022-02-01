#include "basic.hxx"

#include "renderer/containers/camera_data.hxx"

#include "dreco.hxx"

vk_shader_basic_vert::vk_shader_basic_vert()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.vert.spv");
}

vk::PipelineShaderStageCreateInfo vk_shader_basic_vert::getPipelineShaderStageCreateInfo() const
{
	return vk::PipelineShaderStageCreateInfo()
		.setModule(_shaderModule)
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName("main");
}

vk::DescriptorSetLayoutBinding vk_shader_basic_vert::getDescriptorSetLayoutBinding() const
{
	return vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);
}

void vk_shader_basic_vert::addPushConstantRange(std::vector<vk::PushConstantRange>& ranges) const
{
	ranges.push_back(vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(camera_data)));
}

vk_shader_basic_frag::vk_shader_basic_frag()
	: vk_shader()
{
	_shaderPath = DRECO_SHADER("basic.frag.spv");
}

vk::PipelineShaderStageCreateInfo vk_shader_basic_frag::getPipelineShaderStageCreateInfo() const
{
	return vk::PipelineShaderStageCreateInfo()
		.setModule(_shaderModule)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName("main");
}

vk::DescriptorSetLayoutBinding vk_shader_basic_frag::getDescriptorSetLayoutBinding() const
{
	return vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(4)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
}
