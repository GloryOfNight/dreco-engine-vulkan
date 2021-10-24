#pragma once
#include "core/containers/material.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

class vk_descriptor_set;

class vk_graphics_pipeline final
{
public:
	vk_graphics_pipeline() = default;
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = default;
	~vk_graphics_pipeline() { destroy(); };

	void create(const material& mat);

	void recreatePipeline();

	void destroy();

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer);

	const material& getMaterial() const;

	const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;

	vk::PipelineLayout getLayout() const;

	vk::Pipeline get() const;

protected:
	void createDescriptorLayouts(const vk::Device device);

	void createPipelineLayout(const vk::Device device);

	void createPipeline(const vk::Device device);

private:
	material _mat;

	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;

	vk::PipelineLayout _pipelineLayout;

	vk::Pipeline _pipeline;
};