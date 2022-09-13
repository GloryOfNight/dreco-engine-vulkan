#pragma once

#include "vk_shader.hxx"
#include "vk_graphics_pipeline.hxx"

#include <string>
#include <vulkan/vulkan.hpp>
#include <spirv-reflect/spirv_reflect.h>

struct vk_descriptor_write_infos
{
	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	std::vector<vk::DescriptorImageInfo> imageInfos;
};

struct vk_descriptor_shader_data
{
	uint32_t _descriptorSetIndex{UINT32_MAX};

	vk::DescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo{};

	std::vector<vk::DescriptorSetLayoutBinding> _descriptorSetLayoutBindings{};

	std::vector<vk::DescriptorPoolSize> getDescriptorPoolSizes() const;
};

struct vk_vertex_input_info
{
	std::array<vk::VertexInputBindingDescription, 1> _bindingDesc;
	std::vector<vk::VertexInputAttributeDescription> _attributeDesc;
};

class vk_shader
{
public:
	vk_shader() = default;
	~vk_shader();

	void create();

	void destroy();

	bool isValid() const;

	std::string_view getPath() const;

	virtual void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const {};

	virtual void cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh) const{};

	vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const;

	const std::vector<vk_descriptor_shader_data>& getDescirptorShaderData() const;
	
	const std::vector<vk::PushConstantRange>& getPushConstantRanges() const;

	const vk_vertex_input_info getVertexInputInfo() const;

protected:
	void descriptShader(const std::string_view& shaderCode);

	std::string _shaderPath{};

	vk::ShaderModule _shaderModule{};

	SpvReflectShaderModule _reflModule{};

	std::vector<vk_descriptor_shader_data> _descirptorShaderData;

	std::vector<vk::PushConstantRange> _pushConstantRanges;
};