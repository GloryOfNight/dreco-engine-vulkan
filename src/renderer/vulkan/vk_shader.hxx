#pragma once

#include "vk_shader.hxx"

#include <string>
#include <vulkan/vulkan.hpp>

class vk_shader
{
public:
	vk_shader() = default;
	~vk_shader();

	void create();

	void destroy();

	bool isValid() const;

	std::string_view getPath() const;

	virtual vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo() const = 0;

	virtual vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() const = 0;

protected:
	std::string _shaderPath{};

	vk::ShaderModule _shaderModule{};

private:
	vk::ShaderModule loadShader(const std::string_view& path, const vk::Device device);
};