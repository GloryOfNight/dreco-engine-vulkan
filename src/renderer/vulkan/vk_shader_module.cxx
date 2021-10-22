#include "vk_shader_module.hxx"

#include "vk_renderer.hxx"
#include "vk_utils.hxx"

vk_shader_module::vk_shader_module()
	: _shaderModule{}
{
}

vk_shader_module::~vk_shader_module()
{
	destroy();
}

void vk_shader_module::create(const uint32_t* code, const size_t& size)
{
	const vk::Device device = vk_renderer::get()->getDevice();
	_shaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, size, code));
}

void vk_shader_module::destroy()
{
	if (_shaderModule)
	{
		const vk::Device device = vk_renderer::get()->getDevice();
		device.destroyShaderModule(_shaderModule);
	}
}