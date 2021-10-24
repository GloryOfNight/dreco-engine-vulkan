#pragma once
#include <vulkan/vulkan.hpp>

class vk_shader_module final
{
public:
	vk_shader_module() = default;
	vk_shader_module(const vk_shader_module&) = delete;
	vk_shader_module(vk_shader_module&&) = default;
	~vk_shader_module() { destroy(); };

	void create(const uint32_t* code, const size_t& size);

	vk::ShaderModule get() const { return _shaderModule; };

	void destroy();

private:
	vk::ShaderModule _shaderModule;
};