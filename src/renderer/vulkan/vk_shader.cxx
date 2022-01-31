#include "vk_shader.hxx"

#include "core/utils/file_utils.hxx"

void vk_shader::Create(vk::Device device)
{
	_vertexShaderModule = loadShader(_vertexShaderPath, device);
	_fragmentShaderModule = loadShader(_fragmentShaderPath, device);

	const auto bindings = getDescriptorSetLayoutBindings();
	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
															 .setFlags({})
															 .setBindings(bindings);
	_descriptorSetLayout = device.createDescriptorSetLayout(layoutCreateInfo);
}

void vk_shader::Destroy(vk::Device device)
{
	device.destroyShaderModule(_vertexShaderModule);
	device.destroyShaderModule(_fragmentShaderModule);
	device.destroyDescriptorSetLayout(_descriptorSetLayout);
}

bool vk_shader::isValid() const
{
	return _descriptorSetLayout;
}

vk::DescriptorSetLayout vk_shader::getDescriptorSetLayout() const
{
	return _descriptorSetLayout;
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
