#pragma once

#include "vk_shader.hxx"

#include <string>
#include <vulkan/vulkan.hpp>

class vk_shader
{
public:
	vk_shader() = default;

	void Create(vk::Device device);

	void Destroy(vk::Device device);

	bool isValid() const;

	vk::DescriptorSetLayout getDescriptorSetLayout() const;

	virtual std::vector<vk::PipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos() const = 0;

	virtual std::vector<vk::DescriptorSetLayoutBinding> getDescriptorSetLayoutBindings() const = 0;

protected:
	std::string _vertexShaderPath{};

	std::string _fragmentShaderPath{};

	vk::ShaderModule _vertexShaderModule{};

	vk::ShaderModule _fragmentShaderModule{};

	vk::DescriptorSetLayout _descriptorSetLayout{};

private:
	vk::ShaderModule loadShader(const std::string_view& path, const vk::Device device);
};