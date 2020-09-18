#pragma once
#include <vulkan/vulkan.h>

class vk_shader_module
{
public:
	vk_shader_module();
	~vk_shader_module();

	void create(VkDevice vkDevice, const char* code, const size_t& code_size);

	VkShaderModule get() const;

	void destroy();

private:
	VkDevice _vkDevice;

	VkShaderModule _vkShaderModule;
};