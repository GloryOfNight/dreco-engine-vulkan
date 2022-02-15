#include "vk_graphics_pipeline.hxx"

#include "core/utils/file_utils.hxx"
#include "math/vec3.hxx"
#include "renderer/containers/vk_vertex.hxx"
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

	_vertShader = renderer->findShader<vk_shader_basic_vert>();
	_fragShader = renderer->findShader<vk_shader_basic_frag>();

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
		_vertShader->cmdPushConstants(commandBuffer, getLayout(), mesh);
		mesh->bindToCmdBuffer(commandBuffer);
	}
}

void vk_graphics_pipeline::updateDescriptiors()
{
	vk_descriptor_write_infos infos;
	_vertShader->addDescriptorWriteInfos(infos, *this);
	_fragShader->addDescriptorWriteInfos(infos, *this);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	_vertShader->addDescriptorPoolSizes(poolSizes);
	_fragShader->addDescriptorPoolSizes(poolSizes);

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

	_doubleSided = mat._doubleSided;

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
	_vertShader->addDescriptorSetLayoutBindings(bindings);
	_fragShader->addDescriptorSetLayoutBindings(bindings);

	_descriptorSetLayout = device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo()
																.setFlags({})
																.setBindings(bindings));

	std::vector<vk::DescriptorPoolSize> poolSizes;
	_vertShader->addDescriptorPoolSizes(poolSizes);
	_fragShader->addDescriptorPoolSizes(poolSizes);

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
	_vertShader->addPushConstantRange(pushConstantRanges);
	_fragShader->addPushConstantRange(pushConstantRanges);

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(_descriptorSetLayout)
			.setPushConstantRanges(pushConstantRanges);

	_pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

void vk_graphics_pipeline::createPipeline(vk::Device device)
{
	const vk_renderer* renderer{vk_renderer::get()};
	const vk::RenderPass renderPass{renderer->getRenderPass()};
	const vk::SurfaceKHR surface = renderer->getSurface();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();
	const auto physicaDeviceFeatures = physicalDevice.getFeatures();

	const vk::Extent2D extent = renderer->getCurrentExtent();
	const vk::SampleCountFlagBits sampleCount = renderer->getSettings().getPrefferedSampleCount();

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo;
	_vertShader->addPipelineShaderStageCreateInfo(shaderStagesInfo);
	_fragShader->addPipelineShaderStageCreateInfo(shaderStagesInfo);

	const auto vertexInputBindingDescription{vk_vertex::getInputBindingDescription()};
	const auto vertexInputAttributeDescriptions{vk_vertex::getInputAttributeDescription()};

	const vk::PipelineVertexInputStateCreateInfo vertexInputState =
		vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(vertexInputBindingDescription)
			.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);

	const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

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

	const vk::PipelineRasterizationStateCreateInfo rasterizationState =
		vk::PipelineRasterizationStateCreateInfo()
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(renderer->getSettings().getDefaultPolygonMode())
			.setLineWidth(1.0F)
			.setCullMode(_doubleSided ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthClampEnable(VK_FALSE)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0.0F)
			.setDepthBiasSlopeFactor(0.0F)
			.setDepthBiasClamp(0.0F);

	const vk::PipelineMultisampleStateCreateInfo multisampleState =
		vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(VK_FALSE)
			.setRasterizationSamples(sampleCount)
			.setMinSampleShading(1.0F)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(VK_TRUE)
			.setAlphaToOneEnable(physicaDeviceFeatures.alphaToOne);

	const vk::PipelineColorBlendAttachmentState colorBlendAttachment =
		vk::PipelineColorBlendAttachmentState()
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

	const vk::PipelineColorBlendStateCreateInfo colorBlendingState =
		vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments({1, &colorBlendAttachment})
			.setBlendConstants({0.0F, 0.0F, 0.0F, 0.0F});

	const vk::PipelineDepthStencilStateCreateInfo depthStencil =
		vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(VK_TRUE)
			.setDepthWriteEnable(VK_TRUE)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setMinDepthBounds(0.0F)
			.setMaxDepthBounds(1.0F)
			.setStencilTestEnable(VK_FALSE);

	const vk::GraphicsPipelineCreateInfo pipelineCreateInfo =
		vk::GraphicsPipelineCreateInfo()
			.setStages(shaderStagesInfo)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizationState)
			.setPColorBlendState(&colorBlendingState)
			.setPMultisampleState(&multisampleState)
			.setPDepthStencilState(&depthStencil)
			.setLayout(_pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0);

	const auto createPipelineResult = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	assert(vk::Result::eSuccess == createPipelineResult.result);
	_pipeline = createPipelineResult.value;
}
