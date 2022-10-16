#pragma once

#include "vk_shader.hxx"

#include <spirv-reflect/spirv_reflect.h>
#include <string>
#include <vulkan/vulkan.hpp>

class vk_graphics_pipeline;
class vk_mesh;

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

class vk_shader
{
	struct vk_vertex_input_info
	{
		std::array<vk::VertexInputBindingDescription, 1> _bindingDesc;
		std::vector<vk::VertexInputAttributeDescription> _attributeDesc;
	};

public:
	using shared = std::shared_ptr<vk_shader>;

	vk_shader() = default;
	~vk_shader();

	void create(const std::string_view inShaderPath);

	void destroy();

	bool isValid() const noexcept;

	std::string_view getPath() const noexcept;

	const SpvReflectShaderModule& getRefl() const;

	vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const noexcept;

	std::vector<vk_descriptor_shader_data> getDescirptorShaderData() const noexcept;

	std::vector<vk::PushConstantRange> getPushConstantRanges() const noexcept;

	vk_vertex_input_info getVertexInputInfo() const noexcept;

protected:
	std::string _shaderPath;

	vk::ShaderModule _shaderModule;

	SpvReflectShaderModule _reflModule;
};