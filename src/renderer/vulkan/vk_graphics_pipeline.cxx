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

vk_graphics_pipeline::vk_graphics_pipeline(const vk_scene* scene, const material& mat)
	: _scene{scene}
	, _mat{mat}
{
}

void vk_graphics_pipeline::create()
{
	vk_renderer* renderer{vk_renderer::get()};
	vk::Device device = renderer->getDevice();

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

void vk_graphics_pipeline::bindToCmdBuffer(const vk::CommandBuffer commandBuffer)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, getLayout(), 0, _descriptorSets, nullptr);
	for (const vk_mesh* mesh : _dependedMeshes)
	{
		_vertShader->cmdPushConstants(commandBuffer, getLayout(), mesh);

		commandBuffer.pushConstants(getLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4), &mesh->_mat);
		mesh->bindToCmdBuffer(commandBuffer);
	}
}

void vk_graphics_pipeline::updateDescriptiors()
{
	vk_descriptor_write_infos infos;
	_vertShader->addDescriptorWriteInfos(infos, _scene, _mat);
	_fragShader->addDescriptorWriteInfos(infos, _scene, _mat);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	_vertShader->addDescriptorPoolSizes(poolSizes);
	_fragShader->addDescriptorPoolSizes(poolSizes);

	const size_t writeSize = poolSizes.size();
	std::vector<vk::WriteDescriptorSet> writes(writeSize, vk::WriteDescriptorSet());
	for (size_t i = 0; i < writeSize; ++i)
	{
		writes[i] = vk::WriteDescriptorSet()
						.setDstSet(_descriptorSets[0])
						.setDstBinding(i)
						.setDescriptorType(poolSizes[i].type);
		switch (poolSizes[i].type)
		{
		case vk::DescriptorType::eUniformBuffer:
			writes[i].setBufferInfo(infos.bufferInfos);
			break;
		case vk::DescriptorType::eCombinedImageSampler:
			writes[i].setImageInfo(infos.imageInfos);
			break;
		default:
			break;
		}
	}
	vk_renderer::get()->getDevice().updateDescriptorSets(writes, nullptr);
}

const material& vk_graphics_pipeline::getMaterial() const
{
	return _mat;
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
			.setCullMode(_mat._doubleSided ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack)
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
