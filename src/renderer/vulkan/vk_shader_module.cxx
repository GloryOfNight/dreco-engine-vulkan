#include "vk_shader_module.hxx"

#include "vk_utils.hxx"

vk_shader_module::vk_shader_module()
{
}

vk_shader_module::~vk_shader_module()
{
	destroy();
}

void vk_shader_module::create(VkDevice vkDevice, const char* code, const size_t& codeSize)
{
	_vkDevice = vkDevice;

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = codeSize;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code);

	VK_CHECK(vkCreateShaderModule(_vkDevice, &shaderModuleCreateInfo, VK_NULL_HANDLE, &_vkShaderModule));
}

VkShaderModule vk_shader_module::get() const
{
	return _vkShaderModule;
}

void vk_shader_module::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice && VK_NULL_HANDLE != _vkShaderModule)
	{
		vkDestroyShaderModule(_vkDevice, _vkShaderModule, VK_NULL_HANDLE);
		_vkDevice = VK_NULL_HANDLE;
	}
}