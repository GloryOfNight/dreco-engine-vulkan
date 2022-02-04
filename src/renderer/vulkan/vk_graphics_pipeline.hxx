#pragma once
#include "core/containers/gltf/material.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

class vk_descriptor_set;
class vk_mesh;
class vk_scene;
class vk_shader;

class vk_graphics_pipeline final
{
public:
	vk_graphics_pipeline(const vk_scene* scene, const material& mat);
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = default;
	~vk_graphics_pipeline() { destroy(); };

	void create();

	void recreatePipeline();

	void destroy();

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer);

	void updateDescriptiors();

	const material& getMaterial() const;

	vk::DescriptorSetLayout getDescriptorSetLayout() const;

	vk::PipelineLayout getLayout() const;

	vk::Pipeline get() const;

	void addDependentMesh(const vk_mesh* mesh);

protected:
	void createDescriptorSets(vk::Device device);

	void createPipelineLayout(vk::Device device);

	void createPipeline(vk::Device device);

private:
	const vk_scene* _scene;
	const material _mat;

	vk_shader* _vertShader;

	vk_shader* _fragShader;

	std::vector<const vk_mesh*> _dependedMeshes;

	vk::DescriptorSetLayout _descriptorSetLayout;
	vk::DescriptorPool _descriptorPool;
	std::vector<vk::DescriptorSet> _descriptorSets;

	vk::PipelineLayout _pipelineLayout;

	vk::Pipeline _pipeline;
};