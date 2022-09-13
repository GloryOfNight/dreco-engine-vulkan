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
		for (auto descriptorSetLayout : _descriptorSetLayouts)
			device.destroyDescriptorSetLayout(descriptorSetLayout);

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
	for (auto& shader : _shaders)
	{
		vk_descriptor_write_infos infos;
		shader.second->addDescriptorWriteInfos(infos, *this);
		const auto& shaderDataSets = shader.second->getDescirptorShaderData();
		for (const auto& data : shaderDataSets)
		{
			std::vector<vk::WriteDescriptorSet> writes(data._descriptorSetLayoutBindings.size(), vk::WriteDescriptorSet());
			for (uint32_t i = 0; i < data._descriptorSetLayoutBindings.size(); ++i)
			{
				const auto& binding = data._descriptorSetLayoutBindings[i];
				writes[i] = vk::WriteDescriptorSet()
								.setDstSet(_descriptorSets[data._descriptorSetIndex])
								.setDstBinding(binding.binding)
								.setDescriptorType(binding.descriptorType);
				switch (binding.descriptorType)
				{
				case vk::DescriptorType::eUniformBuffer:
					writes[i].setDescriptorCount(binding.descriptorCount);
					writes[i].setPBufferInfo(&infos.bufferInfos[0]);
					break;
				case vk::DescriptorType::eCombinedImageSampler:
					writes[i].setDescriptorCount(binding.descriptorCount);
					writes[i].setPImageInfo(&infos.imageInfos[0]);
					break;
				default:
					break;
				}
			}
			vk_renderer::get()->getDevice().updateDescriptorSets(writes, nullptr);
		}
	}
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
	uint32_t maxSets{};
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (auto& shader : _shaders)
	{
		const auto& shaderDataSets = shader.second->getDescirptorShaderData();
		maxSets += shaderDataSets.size();
		for (const auto& data : shaderDataSets)
		{
			_descriptorSetLayouts.push_back(device.createDescriptorSetLayout(data._descriptorSetLayoutCreateInfo));
			const auto& dataPoolSizes = data.getDescriptorPoolSizes();

			std::copy(dataPoolSizes.begin(), dataPoolSizes.end(), std::back_inserter(poolSizes));
		}
	}

	_descriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
													  .setFlags({})
													  .setPoolSizes(poolSizes)
													  .setMaxSets(maxSets));

	_descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
														.setDescriptorPool(_descriptorPool)
														.setSetLayouts(_descriptorSetLayouts));

	updateDescriptiors();
}

void vk_graphics_pipeline::createPipelineLayout(vk::Device device)
{
	std::vector<vk::PushConstantRange> pushConstantRanges;
	for (auto& shader : _shaders)
	{
		const auto& ranges = shader.second->getPushConstantRanges();
		std::copy(ranges.begin(), ranges.end(), std::back_inserter(pushConstantRanges));
	}

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(_descriptorSetLayouts)
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
		shaderStages.push_back(shader.second->getPipelineShaderStageCreateInfo());
	}

	const auto vertexInputInfo  = _shaders[vk::ShaderStageFlagBits::eVertex]->getVertexInputInfo();
	const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
									  .setVertexBindingDescriptions(vertexInputInfo._bindingDesc)
									  .setVertexAttributeDescriptions(vertexInputInfo._attributeDesc);

	std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments;
	colorBlendAttachments[0] = vk::PipelineColorBlendAttachmentState()
								   .setBlendEnable(VK_TRUE)
								   .setColorWriteMask(
									   vk::ColorComponentFlagBits::eR |
									   vk::ColorComponentFlagBits::eG |
									   vk::ColorComponentFlagBits::eB |
									   vk::ColorComponentFlagBits::eA)
								   .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
								   .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
								   .setColorBlendOp(vk::BlendOp::eAdd)
								   .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
								   .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
								   .setAlphaBlendOp(vk::BlendOp::eSubtract);

	const std::array<float, 4> colorBlendConstants = {0.F, 0.F, 0.F, 0.F};

	const auto colorBlendingState = vk::PipelineColorBlendStateCreateInfo()
										.setLogicOpEnable(VK_FALSE)
										.setLogicOp(vk::LogicOp::eCopy)
										.setAttachments(colorBlendAttachments)
										.setBlendConstants(colorBlendConstants);

	const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
										.setTopology(vk::PrimitiveTopology::eTriangleList)
										.setPrimitiveRestartEnable(VK_FALSE);

	const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
										.setRasterizerDiscardEnable(VK_FALSE)
										.setPolygonMode(vk::PolygonMode::eFill)
										.setLineWidth(1.0F)
										.setCullMode(vk::CullModeFlagBits::eNone)
										.setFrontFace(vk::FrontFace::eCounterClockwise)
										.setDepthClampEnable(VK_FALSE)
										.setDepthBiasEnable(VK_FALSE)
										.setDepthBiasConstantFactor(0.0F)
										.setDepthBiasSlopeFactor(0.0F)
										.setDepthBiasClamp(0.0F);

	const auto multisamplingState = vk::PipelineMultisampleStateCreateInfo()
										.setSampleShadingEnable(VK_FALSE)
										.setRasterizationSamples(renderer->getSettings().getPrefferedSampleCount())
										.setMinSampleShading(1.0F)
										.setPSampleMask(nullptr)
										.setAlphaToCoverageEnable(VK_TRUE)
										.setAlphaToOneEnable(VK_TRUE);

	const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
									   .setDepthTestEnable(VK_TRUE)
									   .setDepthWriteEnable(VK_TRUE)
									   .setDepthCompareOp(vk::CompareOp::eLess)
									   .setDepthBoundsTestEnable(VK_FALSE)
									   .setMinDepthBounds(0.0F)
									   .setMaxDepthBounds(1.0F)
									   .setStencilTestEnable(VK_TRUE);

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
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizationState)
			.setPColorBlendState(&colorBlendingState)
			.setPMultisampleState(&multisamplingState)
			.setPDepthStencilState(&depthStencilState)
			.setLayout(_pipelineLayout)
			.setRenderPass(renderer->getRenderPass())
			.setSubpass(0);

	const auto createPipelineResult = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	assert(vk::Result::eSuccess == createPipelineResult.result);
	_pipeline = createPipelineResult.value;
}
