#pragma once
#include "core/containers/gltf/material.hxx"

#include "vk_shader.hxx"

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

	vk::DescriptorSetLayout getDescriptorSetLayout() const;

	vk::PipelineLayout getLayout() const;

	vk::Pipeline get() const;

protected:
	void createPipelineLayout(const vk::Device device);

	void createPipeline(const vk::Device device);

private:
	material _mat;

	const vk_shader* vertShader;

	const vk_shader* fragShader;

	vk::DescriptorSetLayout _descriptorSetLayout;

	vk::PipelineLayout _pipelineLayout;

	vk::Pipeline _pipeline;
};