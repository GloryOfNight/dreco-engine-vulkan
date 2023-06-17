#include "material.hxx"

#include "dreco.hxx"
#include "renderer.hxx"

de::vulkan::material::~material()
{
	auto device = renderer::get()->getDevice();
	if (_descriptorPool)
	{
		for (auto layout : _descriptorSetLayouts)
			device.destroyDescriptorSetLayout(layout);
		device.destroyDescriptorPool(_descriptorPool);
	}

	_pipelines.clear();

	if (_pipelineLayout)
	{
		device.destroyPipelineLayout(_pipelineLayout);
	}
}

void de::vulkan::material::viewAdded(uint32_t viewIndex)
{
	_pipelines.emplace(viewIndex, createPipeline(viewIndex));
}

void de::vulkan::material::viewUpdated(uint32_t viewIndex)
{
	_pipelines.at(viewIndex) = createPipeline(viewIndex);
}

void de::vulkan::material::viewRemoved(uint32_t viewIndex)
{
	_pipelines.erase(viewIndex);
}

de::vulkan::material::unique de::vulkan::material::makeNew(shader::shared vert, shader::shared frag)
{
	auto mat = unique(new material());
	mat->setShaderVert(vert);
	mat->setShaderFrag(frag);
	return mat;
}

de::vulkan::material_instance* de::vulkan::material::makeInstance()
{
	try
	{
		auto& inst = _instances.emplace_back(material_instance::unique(new material_instance(this)));
		return inst.get();
	}
	catch (vk::OutOfPoolMemoryError)
	{
		DE_LOG(Error, "%s: Out of pool memory", __FUNCTION__);
		resizeDescriptorPool(_instances.size() * 2);
		return makeInstance();
	}
}

void de::vulkan::material::init(size_t maxInstances)
{
	createDescriptorPool(maxInstances);
	createPipelineLayout();

	const auto& views = renderer::get()->getViews();
	for (size_t i = 0; i < views.size(); ++i)
	{
		if (views[i] != nullptr && views[i]->isInitialized())
			viewAdded(i);
	}
}

void de::vulkan::material::setDynamicStates(std::vector<vk::DynamicState>&& dynamicStates)
{
	_pipelineDynamicStates = dynamicStates;
}

void de::vulkan::material::setShaderVert(const shader::shared& inShader)
{
	_vert = inShader;
}

void de::vulkan::material::setShaderFrag(const shader::shared& inShader)
{
	_frag = inShader;
}

void de::vulkan::material::resizeDescriptorPool(uint32_t newSize)
{
	auto device = renderer::get()->getDevice();

	for (auto layout : _descriptorSetLayouts)
		device.destroyDescriptorSetLayout(layout);
	_descriptorSetLayouts.clear();

	device.destroyDescriptorPool(_descriptorPool);

	createDescriptorPool(newSize);

	const auto instancesCount = _instances.size();
	_instances.clear();
	for (size_t i = 0; i < instancesCount; ++i)
		makeInstance();
}

void de::vulkan::material::bindCmd(vk::CommandBuffer commandBuffer) const
{
	auto renderer = renderer::get();
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.at(renderer->getCurrentDrawViewIndex()).get());
}

void de::vulkan::material::createDescriptorPool(uint32_t maxSets)
{
	auto device = renderer::get()->getDevice();
	std::vector<shader::descripted_data> shadersDataSets;
	{
		auto dataSets = _vert->getDescirptorShaderData();
		std::move(dataSets.begin(), dataSets.end(), std::back_inserter(shadersDataSets));
	}
	{
		auto dataSets = _frag->getDescirptorShaderData();
		std::move(dataSets.begin(), dataSets.end(), std::back_inserter(shadersDataSets));
	}

	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto& data : shadersDataSets)
	{
		_descriptorSetLayouts.push_back(device.createDescriptorSetLayout(data._descriptorSetLayoutCreateInfo));

		const auto dataPoolSizes = data.getDescriptorPoolSizes();
		std::move(dataPoolSizes.begin(), dataPoolSizes.end(), std::back_inserter(poolSizes));
	}
	_descriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
													  .setPoolSizes(poolSizes)
													  .setMaxSets(_descriptorSetLayouts.size() * maxSets));
}

void de::vulkan::material::createPipelineLayout()
{
	auto device = renderer::get()->getDevice();

	const auto ranges = getPushConstantRanges();

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(_descriptorSetLayouts)
			.setPushConstantRanges(ranges);

	_pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

const std::vector<vk::DescriptorSetLayout>& de::vulkan::material::getDescriptorSetLayouts() const
{
	return _descriptorSetLayouts;
}

vk::DescriptorPool de::vulkan::material::getDescriptorPool() const
{
	return _descriptorPool;
}

std::vector<vk::PushConstantRange> de::vulkan::material::getPushConstantRanges() const
{
	std::vector<vk::PushConstantRange> out;
	{
		auto ranges = _vert->getPushConstantRanges();
		std::move(ranges.begin(), ranges.end(), std::back_inserter(out));
	}
	{
		auto ranges = _frag->getPushConstantRanges();
		std::move(ranges.begin(), ranges.end(), std::back_inserter(out));
	}
	return out;
}

std::vector<vk::PipelineShaderStageCreateInfo> de::vulkan::material::getShaderStages() const
{
	auto out = std::vector<vk::PipelineShaderStageCreateInfo>(2, vk::PipelineShaderStageCreateInfo());
	out[0] = _vert->getPipelineShaderStageCreateInfo();
	out[1] = _frag->getPipelineShaderStageCreateInfo();
	return out;
}

const de::vulkan::shader::shared& de::vulkan::material::getVertShader() const
{
	return _vert;
}

const de::vulkan::shader::shared& de::vulkan::material::getFragShader() const
{
	return _frag;
}

vk::PipelineLayout de::vulkan::material::getPipelineLayout() const
{
	return _pipelineLayout;
}

vk::UniquePipeline de::vulkan::material::createPipeline(uint32_t viewIndex)
{
	const renderer* renderer{renderer::get()};
	auto view = renderer->getView(viewIndex);

	const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
		{
			_vert->getPipelineShaderStageCreateInfo(),
			_frag->getPipelineShaderStageCreateInfo(),
		};

	const auto vertexInputInfo = _vert->getVertexInputInfo();
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
										.setPolygonMode(view->getSettings().getPolygonMode())
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
										.setRasterizationSamples(view->getSettings().getSampleCount())
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

	const vk::Extent2D extent = view->getCurrentExtent();
	const vk::Viewport viewport =
		vk::Viewport()
			.setX(0)
			.setY(0)
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setMinDepth(0.0F)
			.setMaxDepth(1.0F);

	const vk::Rect2D scissors =
		vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(extent);

	const vk::PipelineViewportStateCreateInfo viewportState =
		vk::PipelineViewportStateCreateInfo()
			.setViewports(viewport)
			.setScissors(scissors);

	const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo()
																.setDynamicStates(_pipelineDynamicStates);

	const vk::GraphicsPipelineCreateInfo pipelineCreateInfo =
		vk::GraphicsPipelineCreateInfo()
			.setStages(shaderStages)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPViewportState(&viewportState)
			.setPDynamicState(&dynamicState)
			.setPRasterizationState(&rasterizationState)
			.setPColorBlendState(&colorBlendingState)
			.setPMultisampleState(&multisamplingState)
			.setPDepthStencilState(&depthStencilState)
			.setLayout(_pipelineLayout)
			.setRenderPass(view->getRenderPass())
			.setSubpass(0);

	auto createPipelineResult = renderer->getDevice().createGraphicsPipelineUnique(nullptr, pipelineCreateInfo);
	assert(vk::Result::eSuccess == createPipelineResult.result);
	return std::move(createPipelineResult.value);
}