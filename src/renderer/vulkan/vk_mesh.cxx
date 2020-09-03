#include "vk_mesh.hxx"
#include "vk_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_physical_device.hxx"
#include "vk_utils.hxx"
#include "vk_shader_module.hxx"
#include "core/utils/file_utils.hxx"

vk_mesh::vk_mesh()
	: _vkDevice{VK_NULL_HANDLE}
	, _vkDescriptorPool{VK_NULL_HANDLE}
	, _vkDescriptorSetLayout{VK_NULL_HANDLE}
	, _vkPipelineLayout{VK_NULL_HANDLE}
	, _vkGraphicsPipeline{VK_NULL_HANDLE}

{
}

vk_mesh::~vk_mesh()
{
	destroy();
}

void vk_mesh::create(const vk_mesh_create_info& create_info)
{
	_mesh = mesh_data::createSprite(); 
	_vkDevice = create_info.device->get();

	createVertexBuffer(create_info.device, create_info.queueFamily, create_info.physicalDevice);
	createIndexBuffer(create_info.device, create_info.queueFamily, create_info.physicalDevice);
	createUniformBuffers(create_info.device, create_info.queueFamily, create_info.physicalDevice, create_info.imageCount);

	createDescriptorPool(create_info.imageCount);
	createDescriptorSetLayot();
	createDescriptorSets(create_info.imageCount);

	createGraphicsPipelineLayout();
	createGraphicsPipeline(create_info.vkRenderPass, create_info.vkExtent);
}

void vk_mesh::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		vkDestroyDescriptorSetLayout(_vkDevice, _vkDescriptorSetLayout, VK_NULL_HANDLE);
		vkDestroyDescriptorPool(_vkDevice, _vkDescriptorPool, VK_NULL_HANDLE);

		vkDestroyPipeline(_vkDevice, _vkGraphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(_vkDevice, _vkPipelineLayout, VK_NULL_HANDLE);

		_vertexBuffer.destroy();
		_indexBuffer.destroy();
		_uniformBuffers.clear();

		_vkDevice = VK_NULL_HANDLE;
	}
}

void vk_mesh::bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer, const uint32_t imageIndex)
{
	vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vkGraphicsPipeline);

	VkBuffer buffers[1]{_vertexBuffer.get()};
	VkDeviceSize offsets[1]{0};
	vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(vkCommandBuffer, _indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vkPipelineLayout, 0, 1,&_vkDescriptorSets[imageIndex], 0, nullptr);
	vkCmdDrawIndexed(vkCommandBuffer, static_cast<uint32_t>(_mesh._indexes.size()), 1, 0, 0, 0);
}

void vk_mesh::beforeSubmitUpdate(const uint32_t imageIndex)
{
	_ubo._model = mat4::makeRotation(vec3(0, 0, 0));
	_ubo._view = mat4::makeTranslation(vec3{0, 0, 1.3f});
	int w, h;
	_ubo._projection = mat4::makeProjection(-1, 1, static_cast<float>(800) / static_cast<float>(800), 75.f);
	// ubo._projection._mat[1][1] = -1;

	_uniformBuffers[imageIndex].map(&_ubo, sizeof(_ubo));
}

void vk_mesh::createDescriptorPool(const uint32_t imageCount)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(imageCount);

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = imageCount;

	VK_CHECK(vkCreateDescriptorPool(_vkDevice, &poolCreateInfo, VK_NULL_HANDLE, &_vkDescriptorPool));
}

void vk_mesh::createDescriptorSetLayot()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &uboLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(_vkDevice, &layoutCreateInfo, VK_NULL_HANDLE, &_vkDescriptorSetLayout));
}

void vk_mesh::createDescriptorSets(const uint32_t imageCount)
{
	std::vector<VkDescriptorSetLayout> layouts(imageCount, _vkDescriptorSetLayout);
	_vkDescriptorSets.resize(imageCount);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = _vkDescriptorPool;
	allocInfo.descriptorSetCount = imageCount;
	allocInfo.pSetLayouts = layouts.data();

	VK_CHECK(vkAllocateDescriptorSets(_vkDevice, &allocInfo, _vkDescriptorSets.data()));

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _uniformBuffers[i].get();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(uniforms);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.pNext = nullptr;
		descriptorWrite.dstSet = _vkDescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_vkDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void vk_mesh::createGraphicsPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_vkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;
	VK_CHECK(vkCreatePipelineLayout(_vkDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &_vkPipelineLayout));
}

void vk_mesh::createGraphicsPipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent)
{
	size_t vertShaderSize{0};
	char* vertShaderCode = file_utils::read_file("shaders/vert.spv", &vertShaderSize);
	if (nullptr == vertShaderCode)
		throw std::runtime_error("Failed to load binary shader code");
	size_t fragShaderSize{0};
	char* fragShaderCode = file_utils::read_file("shaders/frag.spv", &fragShaderSize);
	if (nullptr == fragShaderCode)
		throw std::runtime_error("Failed to load binary shader code");

	vk_shader_module vertShaderStage;
	vertShaderStage.create(_vkDevice, vertShaderCode, vertShaderSize);

	vk_shader_module fragShaderStage;
	fragShaderStage.create(_vkDevice, fragShaderCode, fragShaderSize);

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

	VkPipelineShaderStageCreateInfo shaderStagesInfo[]{vertShaderStageInfo, fragShaderStageInfo};

	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(vec3);

	VkVertexInputAttributeDescription vertexInputAttributeDescription{};
	vertexInputAttributeDescription.binding = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vkExtent.width);
	viewport.height = static_cast<float>(vkExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

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
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStagesInfo;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = _vkPipelineLayout;
	pipelineInfo.renderPass = vkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &_vkGraphicsPipeline));

	delete[] vertShaderCode;
	delete[] fragShaderCode;
}

void vk_mesh::createVertexBuffer(
	const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::VERTEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(_mesh._vertexes[0]) * _mesh._vertexes.size();

	_vertexBuffer.create(device, buffer_create_info);
	_vertexBuffer.map(_mesh._vertexes.data(), buffer_create_info.size);
}

void vk_mesh::createIndexBuffer(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::INDEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(_mesh._indexes[0]) * _mesh._indexes.size();

	_indexBuffer.create(device, buffer_create_info);
	_indexBuffer.map(_mesh._indexes.data(), buffer_create_info.size);
}

void vk_mesh::createUniformBuffers(const vk_device* device, const vk_queue_family* queueFamily,
	const vk_physical_device* physicalDevice, uint32_t imageCount)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::UNIFORM;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(uniforms);

	_uniformBuffers.resize(imageCount);
	for (auto& buffer : _uniformBuffers)
	{
		buffer.create(device, buffer_create_info);
	}
}
