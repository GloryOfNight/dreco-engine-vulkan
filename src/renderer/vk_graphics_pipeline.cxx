#include "vk_graphics_pipeline.hxx"

#include "core/utils/file_utils.hxx"
#include "math/vec3.hxx"
#include "shaders/basic.hxx"

#include "dreco.hxx"
#include "vk_mesh.hxx"
#include "vk_renderer.hxx"
#include "vk_shader.hxx"
#include "vk_utils.hxx"

#include <array>
#include <vector>

void vk_graphics_pipeline::create(const vk_scene* scene, const gltf::material& mat)
{
	loadGltfMaterial(scene, mat);
	vk_renderer* renderer{vk_renderer::get()};
	vk::Device device = renderer->getDevice();

	vk_buffer::create_info bufferCreateInfo{};
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bufferCreateInfo.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	bufferCreateInfo.size = sizeof(material_data);

	_materialBuffer.create(bufferCreateInfo);
	_materialBuffer.getDeviceMemory().map(&_material, sizeof(material_data));

	_shaders.emplace(vk::ShaderStageFlagBits::eVertex, renderer->findShader<vk_shader_basic_vert>());
	_shaders.emplace(vk::ShaderStageFlagBits::eFragment, renderer->findShader<vk_shader_basic_frag>());

	_settings.setup();
	_settings._rasterizationState.setCullMode(mat._doubleSided ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack);
	_settings._multisamplingState.setRasterizationSamples(renderer->getSettings().getPrefferedSampleCount());

	createDescriptorSets(device);

	createPipelineLayout(device);
	createPipeline(device);
}

void vk_graphics_pipeline::recreatePipeline()
{
	vk_renderer* renderer{vk_renderer::get()};
	const vk::Device device = renderer->getDevice();

	device.destroyPipeline(_pipeline);

	createPipeline(device);
}

void vk_graphics_pipeline::destroy()
{
	const vk::Device device = vk_renderer::get()->getDevice();
	if (_pipeline)
	{
		device.destroyDescriptorSetLayout(_descriptorSetLayout);
		device.destroyDescriptorPool(_descriptorPool);
		device.destroyPipelineLayout(_pipelineLayout);
		device.destroyPipeline(_pipeline);
	}
}

void vk_graphics_pipeline::bindCmd(vk::CommandBuffer commandBuffer)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, getLayout(), 0, _descriptorSets, nullptr);
}

void vk_graphics_pipeline::drawCmd(vk::CommandBuffer commandBuffer)
{
	for (const vk_mesh* mesh : _dependedMeshes)
	{
		for (auto& shader : _shaders)
		{
			shader.second->cmdPushConstants(commandBuffer, getLayout(), mesh);
		}
		mesh->bindToCmdBuffer(commandBuffer);
	}
}

void vk_graphics_pipeline::updateDescriptiors()
{
	vk_descriptor_write_infos infos;
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (auto& shader : _shaders)
	{
		shader.second->addDescriptorWriteInfos(infos, *this);
		shader.second->addDescriptorPoolSizes(poolSizes);
	}

	const size_t writeSize = poolSizes.size();
	std::vector<vk::WriteDescriptorSet> writes(writeSize, vk::WriteDescriptorSet());

	uint32_t bufferOffset{0};
	uint32_t imageOffset{0};
	for (size_t i = 0; i < writeSize; ++i)
	{
		writes[i] = vk::WriteDescriptorSet()
						.setDstSet(_descriptorSets[0])
						.setDstBinding(i)
						.setDescriptorType(poolSizes[i].type);
		switch (poolSizes[i].type)
		{
		case vk::DescriptorType::eUniformBuffer:
			writes[i].setDescriptorCount(poolSizes[i].descriptorCount);
			writes[i].setPBufferInfo(&infos.bufferInfos[bufferOffset]);
			bufferOffset += poolSizes[i].descriptorCount;
			break;
		case vk::DescriptorType::eCombinedImageSampler:
			writes[i].setDescriptorCount(poolSizes[i].descriptorCount);
			writes[i].setPImageInfo(&infos.imageInfos[imageOffset]);
			imageOffset += poolSizes[i].descriptorCount;
			break;
		default:
			break;
		}
	}
	vk_renderer::get()->getDevice().updateDescriptorSets(writes, nullptr);
}

const material_data& vk_graphics_pipeline::getMaterial() const
{
	return _material;
}

const vk_texture_image& vk_graphics_pipeline::getTextureImageFromIndex(uint32_t index) const
{
	if (index < _textures.size() && _textures[index]->isValid())
	{
		return *_textures[index];
	}
	return vk_renderer::get()->getTextureImagePlaceholder();
}

const vk_buffer& vk_graphics_pipeline::getMaterialBuffer() const
{
	return _materialBuffer;
}

vk::DescriptorSetLayout vk_graphics_pipeline::getDescriptorSetLayout() const
{
	return _descriptorSetLayout;
}

vk::PipelineLayout vk_graphics_pipeline::getLayout() const
{
	return _pipelineLayout;
}

vk::Pipeline vk_graphics_pipeline::get() const
{
	return _pipeline;
}

void vk_graphics_pipeline::addDependentMesh(const vk_mesh* mesh)
{
	_dependedMeshes.push_back(mesh);
}

void vk_graphics_pipeline::loadGltfMaterial(const vk_scene* scene, const gltf::material& mat)
{
	_material = material_data();
	_textures.reserve(4);

	_material._baseColorFactor[0] = mat._pbrMetallicRoughness._baseColorFactor[0];
	_material._baseColorFactor[1] = mat._pbrMetallicRoughness._baseColorFactor[1];
	_material._baseColorFactor[2] = mat._pbrMetallicRoughness._baseColorFactor[2];
	_material._baseColorFactor[3] = mat._pbrMetallicRoughness._baseColorFactor[3];

	_material._emissiveFactor[0] = mat._emissive._factor[0];
	_material._emissiveFactor[1] = mat._emissive._factor[1];
	_material._emissiveFactor[2] = mat._emissive._factor[2];

	_material._metallicFactor = mat._pbrMetallicRoughness._metallicFactor;
	_material._roughnessFactor = mat._pbrMetallicRoughness._roughnessFactor;

	_material._normalScale = mat._normal._scale;

	if (mat._pbrMetallicRoughness._baseColorTexture._index != UINT32_MAX)
	{
		_textures.push_back(scene->getTextureImages()[mat._pbrMetallicRoughness._baseColorTexture._index].get());
		_material._baseColorIndex = _textures.size() - 1;
	}
	if (mat._pbrMetallicRoughness._metallicRoughnessTexture._index != UINT32_MAX)
	{
		_textures.push_back(scene->getTextureImages()[mat._pbrMetallicRoughness._metallicRoughnessTexture._index].get());
		_material._metallicRoughnessIndex = _textures.size() - 1;
	}
	if (mat._normal._index != UINT32_MAX)
	{
		_textures.push_back(scene->getTextureImages()[mat._normal._index].get());
		_material._normalIndex = _textures.size() - 1;
	}
	if (mat._emissive._index != UINT32_MAX)
	{
		_textures.push_back(scene->getTextureImages()[mat._emissive._index].get());
		_material._emissiveIndex = _textures.size() - 1;
	}
}

void vk_graphics_pipeline::createDescriptorSets(vk::Device device)
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (auto& shader : _shaders)
	{
		shader.second->addDescriptorSetLayoutBindings(bindings);
		shader.second->addDescriptorPoolSizes(poolSizes);
	}

	_descriptorSetLayout = device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo()
																.setFlags({})
																.setBindings(bindings));

	_descriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
													  .setFlags({})
													  .setPoolSizes(poolSizes)
													  .setMaxSets(1));

	_descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
														.setDescriptorPool(_descriptorPool)
														.setSetLayouts(_descriptorSetLayout));

	updateDescriptiors();
}

void vk_graphics_pipeline::createPipelineLayout(vk::Device device)
{
	std::vector<vk::PushConstantRange> pushConstantRanges;
	for (auto& shader : _shaders)
	{
		shader.second->addPushConstantRange(pushConstantRanges);
	}

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(_descriptorSetLayout)
			.setPushConstantRanges(pushConstantRanges);

	_pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

void vk_graphics_pipeline::createPipeline(vk::Device device)
{
	const vk_renderer* renderer{vk_renderer::get()};

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(_shaders.size());
	for (const auto& shader : _shaders)
	{
		shader.second->addPipelineShaderStageCreateInfo(shaderStages);
	}

	const vk::Extent2D extent = renderer->getCurrentExtent();
	const vk::Viewport viewport =
		vk::Viewport()
			.setX(0)
			.setY(0)
			.setWidth(static_cast<float>(extent.width))
			.setHeight(static_cast<float>(extent.height))
			.setMinDepth(0.0F)
			.setMaxDepth(1.0F);

	const vk::Rect2D scissors =
		vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(extent);

	const vk::PipelineViewportStateCreateInfo viewportState =
		vk::PipelineViewportStateCreateInfo()
			.setViewports({1, &viewport})
			.setScissors({1, &scissors});

	const vk::GraphicsPipelineCreateInfo pipelineCreateInfo =
		vk::GraphicsPipelineCreateInfo()
			.setStages(shaderStages)
			.setPVertexInputState(&_settings._vertexInputState)
			.setPInputAssemblyState(&_settings._inputAssemblyState)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&_settings._rasterizationState)
			.setPColorBlendState(&_settings._colorBlendingState)
			.setPMultisampleState(&_settings._multisamplingState)
			.setPDepthStencilState(&_settings._depthStencilState)
			.setLayout(_pipelineLayout)
			.setRenderPass(renderer->getRenderPass())
			.setSubpass(0);

	const auto createPipelineResult = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	assert(vk::Result::eSuccess == createPipelineResult.result);
	_pipeline = createPipelineResult.value;
}
