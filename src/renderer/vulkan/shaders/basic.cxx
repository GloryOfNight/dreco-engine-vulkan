#include "basic.hxx"

#include "dreco.hxx"

vk_shader_basic::vk_shader_basic()
	: vk_shader()
{
	_vertexShaderPath = DRECO_SHADER("basic.vert.spv");
	_fragmentShaderPath = DRECO_SHADER("basic.frag.spv");
}

std::vector<vk::PipelineShaderStageCreateInfo> vk_shader_basic::getPipelineShaderStageCreateInfos() const
{
	const auto vert = vk::PipelineShaderStageCreateInfo()
						  .setModule(_vertexShaderModule)
						  .setStage(vk::ShaderStageFlagBits::eVertex)
						  .setPName("main");

	const auto frag = vk::PipelineShaderStageCreateInfo()
						  .setModule(_fragmentShaderModule)
						  .setStage(vk::ShaderStageFlagBits::eFragment)
						  .setPName("main");

	return {vert, frag};
}

std::vector<vk::DescriptorSetLayoutBinding> vk_shader_basic::getDescriptorSetLayoutBindings() const
{
	const vk::DescriptorSetLayoutBinding uniformBinding =
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	const vk::DescriptorSetLayoutBinding sampledImageBinding =
		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(4)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	return {uniformBinding, sampledImageBinding};
}