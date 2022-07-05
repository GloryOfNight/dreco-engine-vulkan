#include "vk_shader.hxx"

#include "core/utils/file_utils.hxx"
#include "renderer/vk_renderer.hxx"

#include "dreco.hxx"

#include <spirv-reflect/spirv_reflect.h>

vk_shader::~vk_shader()
{
	destroy();
}

void vk_shader::create()
{
	std::string shaderCode;
	file_utils::readFile(_shaderPath, shaderCode);
	if (!shaderCode.empty())
	{
		_shaderModule = vk_renderer::get()->getDevice().createShaderModule(vk::ShaderModuleCreateInfo({}, shaderCode.size(), reinterpret_cast<uint32_t*>(shaderCode.data())));
		descriptShader(shaderCode);
	}
}

void vk_shader::destroy()
{
	if (_shaderModule)
	{
		vk_renderer::get()->getDevice().destroyShaderModule(_shaderModule);
		_shaderModule = nullptr;
	}
}

bool vk_shader::isValid() const
{
	return _shaderModule;
}

std::string_view vk_shader::getPath() const
{
	return _shaderPath;
}

const vk::PipelineShaderStageCreateInfo& vk_shader::getPipelineShaderStageCreateInfo() const
{
	return _pipelineShaderStageCreateInfo;
}

const std::vector<vk_descriptor_shader_data> vk_shader::getDescirptorShaderData() const
{
	return _descirptorShaderData;
}

const std::vector<vk::PushConstantRange> vk_shader::getPushConstantRanges() const
{
	return _pushConstantRanges;
}

void vk_shader::descriptShader(const std::string_view& shaderCode)
{
	SpvReflectShaderModule reflModule;
	const SpvReflectResult result =
		spvReflectCreateShaderModule(shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()), &reflModule);

	// spirv failed to reflect, investigate required
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	_pipelineShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo()
										 .setModule(_shaderModule)
										 .setStage(static_cast<vk::ShaderStageFlagBits>(reflModule.shader_stage))
										 .setPName(reflModule.entry_point_name);

	_descirptorShaderData.resize(reflModule.descriptor_set_count, vk_descriptor_shader_data());
	for (uint8_t i = 0; i < reflModule.descriptor_set_count; i++)
	{
		const auto& reflDescSet = reflModule.descriptor_sets[i];
		auto& descSetData = _descirptorShaderData[i];

		descSetData._descriptorSetIndex = reflDescSet.set;
		descSetData._descriptorSetLayoutBindings.resize(reflDescSet.binding_count, vk::DescriptorSetLayoutBinding());
		for (uint8_t k = 0; k < reflDescSet.binding_count; k++)
		{
			const auto& reflBinding = reflModule.descriptor_bindings[i];
			auto& binding = descSetData._descriptorSetLayoutBindings[i];

			binding = vk::DescriptorSetLayoutBinding()
						  .setBinding(reflBinding.binding)
						  .setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type))
						  .setDescriptorCount(reflBinding.count)
						  .setStageFlags(static_cast<vk::ShaderStageFlagBits>(reflModule.shader_stage));
		}
		descSetData._descriptorSetLayoutCreateInfo.setBindings(descSetData._descriptorSetLayoutBindings);
	}

	_pushConstantRanges.resize(reflModule.push_constant_block_count, vk::PushConstantRange());
	for (uint8_t i = 0; i < reflModule.push_constant_block_count; i++)
	{
		const auto& reflPushConstantRange = reflModule.push_constant_blocks[i];
		auto& pushConstantRange = _pushConstantRanges[i];

		pushConstantRange = vk::PushConstantRange()
								  .setStageFlags(static_cast<vk::ShaderStageFlagBits>(reflModule.shader_stage))
								  .setOffset(reflPushConstantRange.offset)
								  .setSize(reflPushConstantRange.size);
	}

	spvReflectDestroyShaderModule(&reflModule);
}

std::vector<vk::DescriptorPoolSize> vk_descriptor_shader_data::getDescriptorPoolSizes() const
{
	std::vector<vk::DescriptorPoolSize> poolSizes(_descriptorSetLayoutBindings.size(), vk::DescriptorPoolSize());
	for (uint8_t i = 0; i < poolSizes.size(); i++)
	{
		const auto& binding = _descriptorSetLayoutBindings[i];
		poolSizes[i] = vk::DescriptorPoolSize()
						   .setType(binding.descriptorType)
						   .setDescriptorCount(binding.descriptorCount);
	}
	return poolSizes;
}
