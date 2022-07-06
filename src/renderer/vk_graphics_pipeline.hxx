#pragma once
#include "core/containers/gltf/material.hxx"
#include "renderer/containers/material_data.hxx"

#include "vk_buffer.hxx"
#include "vk_graphics_pipeline_settings.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

class vk_descriptor_set;
class vk_mesh;
class vk_scene;
class vk_shader;
class vk_texture_image;

class vk_graphics_pipeline final
{
public:
	vk_graphics_pipeline() = default;
	vk_graphics_pipeline(const vk_graphics_pipeline&) = delete;
	vk_graphics_pipeline(vk_graphics_pipeline&&) = default;
	~vk_graphics_pipeline() { destroy(); };

	void create(const vk_scene* scene, const gltf::material& mat);

	void recreatePipeline();

	void destroy();

	void bindCmd(vk::CommandBuffer commandBuffer);
	void drawCmd(vk::CommandBuffer commandBuffer);

	void updateDescriptiors();

	const material_data& getMaterial() const;

	const vk_texture_image& getTextureImageFromIndex(uint32_t index) const;

	const vk_buffer& getMaterialBuffer() const;

	vk::PipelineLayout getLayout() const;

	vk::Pipeline get() const;

	const vk_graphics_pipeline_settings& getSetings() const { return _settings; };
	vk_graphics_pipeline_settings& getSetings() { return _settings; };

	void addDependentMesh(const vk_mesh* mesh);

protected:
	void loadGltfMaterial(const vk_scene* scene, const gltf::material& mat);

	void createDescriptorSets(vk::Device device);

	void createPipelineLayout(vk::Device device);

	void createPipeline(vk::Device device);

private:
	vk_graphics_pipeline_settings _settings;
	std::map<vk::ShaderStageFlagBits, vk_shader const*> _shaders;

	material_data _material;
	vk_buffer _materialBuffer;

	std::vector<const vk_texture_image*> _textures;

	std::vector<const vk_mesh*> _dependedMeshes;

	vk::DescriptorPool _descriptorPool;
	std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	std::vector<vk::DescriptorSet> _descriptorSets;

	vk::PipelineLayout _pipelineLayout;

	vk::Pipeline _pipeline;
};