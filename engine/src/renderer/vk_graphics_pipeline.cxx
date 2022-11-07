#include "vk_graphics_pipeline.hxx"

#include "core/utils/file_utils.hxx"
#include "math/vec3.hxx"

#include "dreco.hxx"
#include "vk_material.hxx"
#include "vk_mesh.hxx"
#include "vk_renderer.hxx"
#include "vk_shader.hxx"
#include "vk_utils.hxx"

#include <array>
#include <vector>

void vk_graphics_pipeline::create(const vk_material& material)
{
	_owner = &material;

	vk_renderer* renderer{vk_renderer::get()};
	vk::Device device = renderer->getDevice();
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
		device.destroyPipelineLayout(_pipelineLayout);
		device.destroyPipeline(_pipeline);
	}
}

void vk_graphics_pipeline::bindCmd(vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
}

vk::PipelineLayout vk_graphics_pipeline::getLayout() const
{
	return _pipelineLayout;
}

vk::Pipeline vk_graphics_pipeline::get() const
{
	return _pipeline;
}

void vk_graphics_pipeline::createPipelineLayout(vk::Device device)
{
	const auto& descLayouts = _owner->getDescriptorSetLayouts();
	const auto ranges = _owner->getPushConstantRanges();

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descLayouts)
			.setPushConstantRanges(ranges);

	_pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

void vk_graphics_pipeline::createPipeline(vk::Device device)
{
	const vk_renderer* renderer{vk_renderer::get()};

	const auto shaderStages = _owner->getShaderStages();
	const auto vertexInputInfo = _owner->getVertShader()->getVertexInputInfo();
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
