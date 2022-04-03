#include "vk_graphics_pipeline_settings.hxx"

#include "renderer/containers/vk_vertex.hxx"

#include "vk_renderer.hxx"
#include "vk_shader.hxx"

void vk_graphics_pipeline_settings::setup()
{
	auto renderer = vk_renderer::get();
	_vertexInputBindingDesc = vk_vertex::getInputBindingDescription();
	_vertexInputAttributeDesc = vk_vertex::getInputAttributeDescription();

	_inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
							  .setTopology(vk::PrimitiveTopology::eTriangleList)
							  .setPrimitiveRestartEnable(VK_FALSE);

	_rasterizationState = vk::PipelineRasterizationStateCreateInfo()
							  .setRasterizerDiscardEnable(VK_FALSE)
							  .setPolygonMode(vk::PolygonMode::eFill)
							  .setLineWidth(1.0F)
							  .setCullMode(vk::CullModeFlagBits::eBack)
							  .setFrontFace(vk::FrontFace::eCounterClockwise)
							  .setDepthClampEnable(VK_FALSE)
							  .setDepthBiasEnable(VK_FALSE)
							  .setDepthBiasConstantFactor(0.0F)
							  .setDepthBiasSlopeFactor(0.0F)
							  .setDepthBiasClamp(0.0F);

	_multisamplingState = vk::PipelineMultisampleStateCreateInfo()
							  .setSampleShadingEnable(VK_FALSE)
							  .setRasterizationSamples(vk::SampleCountFlagBits::e1)
							  .setMinSampleShading(1.0F)
							  .setPSampleMask(nullptr)
							  .setAlphaToCoverageEnable(VK_TRUE)
							  .setAlphaToOneEnable(VK_TRUE);

	_colorBlendAttachments.clear();
	_colorBlendAttachments.push_back(vk::PipelineColorBlendAttachmentState()
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
										 .setAlphaBlendOp(vk::BlendOp::eSubtract));

	_colorBlendConstants = {0.F, 0.F, 0.F, 0.F};

	_depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
							 .setDepthTestEnable(VK_TRUE)
							 .setDepthWriteEnable(VK_TRUE)
							 .setDepthCompareOp(vk::CompareOp::eLess)
							 .setDepthBoundsTestEnable(VK_FALSE)
							 .setMinDepthBounds(0.0F)
							 .setMaxDepthBounds(1.0F)
							 .setStencilTestEnable(VK_TRUE);

	update();
}

void vk_graphics_pipeline_settings::update()
{
	_vertexInputState = vk::PipelineVertexInputStateCreateInfo()
							.setVertexBindingDescriptions(_vertexInputBindingDesc)
							.setVertexAttributeDescriptions(_vertexInputAttributeDesc);

	_colorBlendingState = vk::PipelineColorBlendStateCreateInfo()
							  .setLogicOpEnable(VK_FALSE)
							  .setLogicOp(vk::LogicOp::eCopy)
							  .setAttachments(_colorBlendAttachments)
							  .setBlendConstants(_colorBlendConstants);
}
