#pragma once
#include "renderer/vk_shader.hxx"

class vk_shader_basic_vert : public vk_shader
{
public:
	vk_shader_basic_vert();

	void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const override;

	void cmdPushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, const vk_mesh* mesh) const override;
};

class vk_shader_basic_frag : public vk_shader
{
public:
	vk_shader_basic_frag();

	void addDescriptorWriteInfos(vk_descriptor_write_infos& infos, const vk_graphics_pipeline& pipeline) const override;
};