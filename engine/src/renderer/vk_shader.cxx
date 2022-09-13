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
	spvReflectDestroyShaderModule(&_reflModule);
}

bool vk_shader::isValid() const
{
	return _shaderModule;
}

std::string_view vk_shader::getPath() const
{
	return _shaderPath;
}

vk::PipelineShaderStageCreateInfo vk_shader::getPipelineShaderStageCreateInfo() const
{
	return vk::PipelineShaderStageCreateInfo()
		.setModule(_shaderModule)
		.setStage(static_cast<vk::ShaderStageFlagBits>(_reflModule.shader_stage))
		.setPName(_reflModule.entry_point_name);
}

const std::vector<vk_descriptor_shader_data>& vk_shader::getDescirptorShaderData() const
{
	return _descirptorShaderData;
}

const std::vector<vk::PushConstantRange>& vk_shader::getPushConstantRanges() const
{
	return _pushConstantRanges;
}

const vk_vertex_input_info vk_shader::getVertexInputInfo() const
{
	vk_vertex_input_info out;
	out._attributeDesc.resize(_reflModule.input_variable_count);

	std::vector<size_t> sizes(_reflModule.input_variable_count, size_t());

	for (int i = 0; i < _reflModule.input_variable_count; ++i)
	{
		const auto& inputVar = _reflModule.input_variables[i];
		out._attributeDesc[inputVar->location] = vk::VertexInputAttributeDescription()
													 .setBinding(0)
													 .setLocation(inputVar->location)
													 .setFormat(static_cast<vk::Format>(inputVar->format));
		sizes[inputVar->location] = (inputVar->numeric.scalar.width / 8) * inputVar->numeric.vector.component_count;
	}

	out._bindingDesc[0] = vk::VertexInputBindingDescription()
							  .setBinding(0)
							  .setInputRate(vk::VertexInputRate::eVertex);
	for (int i = 0; i < _reflModule.input_variable_count; ++i)
	{
		out._bindingDesc[0].stride += sizes[i];
		if (i > 0)
			out._attributeDesc[i].offset = out._attributeDesc[i - 1].offset + sizes[i];
	}

	return out;
}

void vk_shader::descriptShader(const std::string_view& shaderCode)
{
	const SpvReflectResult result =
		spvReflectCreateShaderModule(shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()), &_reflModule);

	// spirv failed to reflect, investigate required
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	_descirptorShaderData.resize(_reflModule.descriptor_set_count, vk_descriptor_shader_data());
	for (uint8_t i = 0; i < _reflModule.descriptor_set_count; i++)
	{
		const auto& reflDescSet = _reflModule.descriptor_sets[i];
		auto& descSetData = _descirptorShaderData[i];

		descSetData._descriptorSetIndex = reflDescSet.set;
		descSetData._descriptorSetLayoutBindings.resize(reflDescSet.binding_count, vk::DescriptorSetLayoutBinding());
		for (uint8_t k = 0; k < reflDescSet.binding_count; k++)
		{
			const auto& reflBinding = _reflModule.descriptor_bindings[k];
			auto& binding = descSetData._descriptorSetLayoutBindings[k];

			binding = vk::DescriptorSetLayoutBinding()
						  .setBinding(reflBinding.binding)
						  .setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type))
						  .setDescriptorCount(reflBinding.count)
						  .setStageFlags(static_cast<vk::ShaderStageFlagBits>(_reflModule.shader_stage));
		}
		descSetData._descriptorSetLayoutCreateInfo.setBindings(descSetData._descriptorSetLayoutBindings);
	}

	_pushConstantRanges.resize(_reflModule.push_constant_block_count, vk::PushConstantRange());
	for (uint8_t i = 0; i < _reflModule.push_constant_block_count; i++)
	{
		const auto& reflPushConstantRange = _reflModule.push_constant_blocks[i];
		auto& pushConstantRange = _pushConstantRanges[i];

		pushConstantRange = vk::PushConstantRange()
								.setStageFlags(static_cast<vk::ShaderStageFlagBits>(_reflModule.shader_stage))
								.setOffset(reflPushConstantRange.offset)
								.setSize(reflPushConstantRange.size);
	}
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
