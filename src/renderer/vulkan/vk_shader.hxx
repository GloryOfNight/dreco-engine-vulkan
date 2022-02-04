#pragma once

#include "vk_shader.hxx"
#include "vk_graphics_pipeline.hxx"

#include <string>
#include <vulkan/vulkan.hpp>

struct vk_descriptor_write_infos
{
	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	std::vector<vk::DescriptorImageInfo> imageInfos;
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

	virtual void addPipelineShaderStageCreateInfo(std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages) const {};

	virtual void addDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings) const {};

	virtual void addPushConstantRange(std::vector<vk::PushConstantRange>& ranges) const {};

	virtual void addDescriptorPoolSizes(std::vector<vk::DescriptorPoolSize>& sizes) const {};

	virtual void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_scene* scene, const material& mat) const {};

	virtual void cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh){};

protected:
	std::string _shaderPath{};

	vk::ShaderModule _shaderModule{};

private:
	vk::ShaderModule loadShader(const std::string_view& path, const vk::Device device);
};