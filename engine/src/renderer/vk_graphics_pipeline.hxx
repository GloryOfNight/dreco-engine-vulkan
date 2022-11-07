#pragma once
#include "core/containers/gltf/material.hxx"

#include "vk_buffer.hxx"
#include "vk_shader.hxx"

#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>

class vk_material;

class vk_graphics_pipeline final
{
public:
	vk_graphics_pipeline() = default;
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = default;
	~vk_graphics_pipeline() { destroy(); };

	void create(const vk_material& material);

	void recreatePipeline();

	void destroy();

	void bindCmd(vk::CommandBuffer commandBuffer) const;

	vk::PipelineLayout getLayout() const;

	vk::Pipeline get() const;

protected:

	void createPipelineLayout(vk::Device device);

	void createPipeline(vk::Device device);

private:
	const vk_material* _owner;

	vk::PipelineLayout _pipelineLayout;

	vk::Pipeline _pipeline;
};