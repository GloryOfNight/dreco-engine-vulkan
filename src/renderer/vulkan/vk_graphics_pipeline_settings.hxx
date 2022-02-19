#pragma once

#include <array>
#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_shader;

struct vk_graphics_pipeline_settings
{
	void default();

	void update();

	std::vector<vk::VertexInputBindingDescription> _vertexInputBindingDesc;

	std::vector<vk::VertexInputAttributeDescription> _vertexInputAttributeDesc;

	vk::PipelineVertexInputStateCreateInfo _vertexInputState;

	vk::PipelineInputAssemblyStateCreateInfo _inputAssemblyState;

	vk::PipelineRasterizationStateCreateInfo _rasterizationState;

	vk::PipelineMultisampleStateCreateInfo _multisamplingState;

	std::vector<vk::PipelineColorBlendAttachmentState> _colorBlendAttachments;

	std::array<float, 4U> _colorBlendConstants;

	vk::PipelineColorBlendStateCreateInfo _colorBlendingState;

	vk::PipelineDepthStencilStateCreateInfo _depthStencilState;
};