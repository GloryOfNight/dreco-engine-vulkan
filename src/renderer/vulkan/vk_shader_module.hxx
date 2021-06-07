#pragma once
#include <vulkan/vulkan.h>

class vk_shader_module final
{
public:
	vk_shader_module();
	vk_shader_module(const vk_shader_module&) = delete;
	vk_shader_module(vk_shader_module&&) = delete;
	~vk_shader_module();

	vk_shader_module& operator=(const vk_shader_module&) = delete;
	vk_shader_module& operator=(const vk_shader_module&&) = delete;

	void create(VkDevice vkDevice, const char* code, const size_t& code_size);

	VkShaderModule get() const;

	void destroy();

private:
	VkDevice _vkDevice;

	VkShaderModule _vkShaderModule;
};