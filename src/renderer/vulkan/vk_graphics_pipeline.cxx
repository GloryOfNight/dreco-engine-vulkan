#include "vk_graphics_pipeline.hxx"

#include "core/utils/file_utils.hxx"
#include "math/vec3.hxx"
#include "renderer/containers/vk_vertex.hxx"

#include "dreco.hxx"
#include "vk_descriptor_set.hxx"
#include "vk_renderer.hxx"
#include "vk_shader_module.hxx"
#include "vk_utils.hxx"

#include <array>
#include <vector>

vk_graphics_pipeline::vk_graphics_pipeline()
	: _pipelineLayout{}
	, _pipeline{}
{
}

vk_graphics_pipeline::~vk_graphics_pipeline()
{
	destroy();
}

void vk_graphics_pipeline::create(const material& mat)
{
	_mat = mat;

	vk_renderer* renderer{vk_renderer::get()};
	const vk::Device device = renderer->getDevice();

	createDescriptorLayouts(device);
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
		for (auto descriptorLayout : _descriptorSetLayouts)
		{
			device.destroyDescriptorSetLayout(descriptorLayout);
		}
		device.destroyPipelineLayout(_pipelineLayout);
		device.destroyPipeline(_pipeline);
	}
}

void vk_graphics_pipeline::bindToCmdBuffer(const vk::CommandBuffer commandBuffer)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
}

const material& vk_graphics_pipeline::getMaterial() const
{
	return _mat;
}

const std::vector<vk::DescriptorSetLayout>& vk_graphics_pipeline::getDescriptorSetLayouts() const
{
	return _descriptorSetLayouts;
}

vk::PipelineLayout vk_graphics_pipeline::getLayout() const
{
	return _pipelineLayout;
}

vk::Pipeline vk_graphics_pipeline::get() const
{
	return _pipeline;
}

void vk_graphics_pipeline::createDescriptorLayouts(const vk::Device device)
{
	const vk::DescriptorSetLayoutBinding uniformBinding =
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	const vk::DescriptorSetLayoutBinding sampledImageBinding =
		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	const std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings{uniformBinding, sampledImageBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutCreateInfo =
		vk::DescriptorSetLayoutCreateInfo()
			.setFlags({})
			.setBindings(layoutBindings);

	_descriptorSetLayouts.push_back(device.createDescriptorSetLayout(layoutCreateInfo));
}

void vk_graphics_pipeline::createPipelineLayout(const vk::Device device)
{
	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(_descriptorSetLayouts)
			.setPushConstantRanges(nullptr);

	_pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

void vk_graphics_pipeline::createPipeline(const vk::Device device)
{
	const vk_renderer* renderer{vk_renderer::get()};
	const vk::RenderPass renderPass{renderer->getRenderPass()};
	const vk::SurfaceKHR surface = renderer->getSurface();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	const vk::Extent2D extent = renderer->getCurrentExtent();
	const vk::SampleCountFlagBits sampleCount = renderer->getSettings().getPrefferedSampleCount();

	std::string vertShaderCode;
	file_utils::readFile(SHADER_BASIC_VERTEX_BIN_URI, vertShaderCode);
	std::string fragShaderCode;
	file_utils::readFile(SHADER_BASIC_FRAGMENT_BIN_URI, fragShaderCode);

	vk_shader_module vertShaderModule;
	vertShaderModule.create(reinterpret_cast<uint32_t*>(vertShaderCode.data()), vertShaderCode.size());

	vk_shader_module fragShaderModule;
	fragShaderModule.create(reinterpret_cast<uint32_t*>(fragShaderCode.data()), fragShaderCode.size());

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo =
		vk::PipelineShaderStageCreateInfo()
			.setModule(vertShaderModule.get())
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setPName("main");

	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
		vk::PipelineShaderStageCreateInfo()
			.setModule(fragShaderModule.get())
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPName("main");

	const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStagesInfo{vertShaderStageInfo, fragShaderStageInfo};

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
			.setAlphaToOneEnable(VK_TRUE);

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
