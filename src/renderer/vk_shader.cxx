#include "vk_shader.hxx"

#include "core/utils/file_utils.hxx"
#include "renderer/vk_renderer.hxx"

#include "dreco.hxx"

vk_shader::~vk_shader()
{
	destroy();
}

void vk_shader::create()
{
	_shaderModule = loadShader(_shaderPath, vk_renderer::get()->getDevice());
}

void vk_shader::destroy()
{
	if (_shaderModule)
	{
		vk_renderer::get()->getDevice().destroyShaderModule(_shaderModule);
		_shaderModule = nullptr;
	}
	
}

bool vk_shader::isValid() const
{
	return _shaderModule;
}

std::string_view vk_shader::getPath() const
{
	return _shaderPath;
}

vk::ShaderModule vk_shader::loadShader(const std::string_view& path, const vk::Device device)
{
	std::string shaderCode;
	file_utils::readFile(path, shaderCode);
	if (!shaderCode.empty())
	{
		return device.createShaderModule(vk::ShaderModuleCreateInfo({}, shaderCode.size(), reinterpret_cast<uint32_t*>(shaderCode.data())));
	}
	return nullptr;
}
