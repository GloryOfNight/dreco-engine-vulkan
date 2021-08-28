#include "vk_graphics_pipeline.hxx"

#include "core/utils/file_utils.hxx"
#include "math/vec3.hxx"
#include "renderer/containers/vertex.hxx"

#include "vk_allocator.hxx"
#include "vk_descriptor_set.hxx"
#include "vk_renderer.hxx"
#include "vk_shader_module.hxx"
#include "vk_utils.hxx"

#include <array>
#include <vector>

vk_graphics_pipeline::vk_graphics_pipeline()
	: _vkPipelineLayout{VK_NULL_HANDLE}
	, _vkPipeline{VK_NULL_HANDLE}
{
}

vk_graphics_pipeline::~vk_graphics_pipeline()
{
	destroy();
}

void vk_graphics_pipeline::create(const vk_descriptor_set& vkDescriptorSet)
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice = renderer->getDevice().get();
	const VkRenderPass vkRenderPass{renderer->getRenderPass()};
	const VkExtent2D vkExtent{renderer->getSurface().getCapabilities().currentExtent};

	createPipelineLayout(vkDevice, vkDescriptorSet);
	createPipeline(vkDevice, vkRenderPass, vkExtent);
}

void vk_graphics_pipeline::recreatePipeline()
{
	vk_renderer* renderer{vk_renderer::get()};
	const VkDevice vkDevice = renderer->getDevice().get();
	const VkRenderPass vkRenderPass{renderer->getRenderPass()};
	const VkExtent2D vkExtent{renderer->getSurface().getCapabilities().currentExtent};

	vkDestroyPipeline(vkDevice, _vkPipeline, vkGetAllocator());

	createPipeline(vkDevice, vkRenderPass, vkExtent);
}

void vk_graphics_pipeline::destroy()
{
	const VkDevice vkDevice = vk_renderer::get()->getDevice().get();
	if (VK_NULL_HANDLE != _vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, _vkPipelineLayout, vkGetAllocator());
		vkDestroyPipeline(vkDevice, _vkPipeline, vkGetAllocator());

		_vkPipelineLayout = VK_NULL_HANDLE;
		_vkPipeline = VK_NULL_HANDLE;
	}
}

VkPipelineLayout vk_graphics_pipeline::getLayout() const
{
	return _vkPipelineLayout;
}

VkPipeline vk_graphics_pipeline::get() const
{
	return _vkPipeline;
}

void vk_graphics_pipeline::createPipelineLayout(const VkDevice vkDevice, const vk_descriptor_set& vkDescriptorSet)
{
	const std::vector<VkDescriptorSetLayout>& vkDescriptorSetLayouts{vkDescriptorSet.getLayouts()};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = vkDescriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = vkDescriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	VK_CHECK(vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &_vkPipelineLayout));
}

void vk_graphics_pipeline::createPipeline(const VkDevice vkDevice, const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent)
{
	const VkSampleCountFlagBits samples = vk_renderer::get()->getPhysicalDevice().getMaxSupportedSampleCount();

	const std::string vertShaderCode = file_utils::read_file("shaders/vert.spv");
	const std::string fragShaderCode = file_utils::read_file("shaders/frag.spv");

	vk_shader_module vertShaderStage;
	vertShaderStage.create(vkDevice, vertShaderCode.data(), vertShaderCode.size());

	vk_shader_module fragShaderStage;
	fragShaderStage.create(vkDevice, fragShaderCode.data(), fragShaderCode.size());

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderStage.get();
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderStage.get();
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStagesInfo{vertShaderStageInfo, fragShaderStageInfo};

	const std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription{vertex::getInputBindingDescription()};
	const std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions{vertex::getInputAttributeDescription()};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = vertexInputBindingDescription.size();
	vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0F;
	viewport.y = 0.0F;
	viewport.width = static_cast<float>(vkExtent.width);
	viewport.height = static_cast<float>(vkExtent.height);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 1.0F;

	VkRect2D scissors{};
	scissors.offset = {0, 0};
	scissors.extent = vkExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0F;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0F;
	rasterizationState.depthBiasClamp = 0.0F;
	rasterizationState.depthBiasSlopeFactor = 0.0F;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = samples;
	multisampleState.minSampleShading = 1.0F;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0F;
	colorBlending.blendConstants[1] = 0.0F;
	colorBlending.blendConstants[2] = 0.0F;
	colorBlending.blendConstants[3] = 0.0F;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStagesInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = _vkPipelineLayout;
	pipelineInfo.renderPass = vkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &_vkPipeline));
}
