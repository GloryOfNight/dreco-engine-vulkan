#include "graphics_pipeline.hxx"

#include "renderer.hxx"

void de::vulkan::graphics_pipeline::create(vk::PipelineLayout pipelineLayout, shader::shared vertShader, shader::shared fragShader)
{
	const renderer* renderer{renderer::get()};

	const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
		{
			vertShader->getPipelineShaderStageCreateInfo(),
			fragShader->getPipelineShaderStageCreateInfo(),
		};

	const auto vertexInputInfo = vertShader->getVertexInputInfo();
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
										.setAlphaToOneEnable(VK_FALSE);

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
			.setLayout(pipelineLayout)
			.setRenderPass(renderer->getRenderPass())
			.setSubpass(0);

	const auto createPipelineResult = renderer::get()->getDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);
	assert(vk::Result::eSuccess == createPipelineResult.result);
	_pipeline = createPipelineResult.value;
}

void de::vulkan::graphics_pipeline::destroy()
{
	const vk::Device device = renderer::get()->getDevice();
	if (_pipeline)
	{
		device.destroyPipeline(_pipeline);
	}
}

void de::vulkan::graphics_pipeline::bindCmd(vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
}

vk::Pipeline de::vulkan::graphics_pipeline::get() const
{
	return _pipeline;
}

void de::vulkan::graphics_pipeline::createPipeline(vk::Device device)
{
}
