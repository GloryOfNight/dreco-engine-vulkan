#pragma once
#include "core/containers/gltf/model.hxx"
#include "vulkan/vulkan.h"

#include "vk_buffer.hxx"

#include <vector>
#include <memory>

class vk_texture_image;
class vk_graphics_pipeline;
class vk_mesh;

class vk_scene
{
public:
	vk_scene() = default;
	~vk_scene();

	void create(const gltf::model& m);

	void recreatePipelines();

	void bindToCmdBuffer(vk::CommandBuffer commandBuffer);

	bool isEmpty() const;

	void destroy();

	std::vector<std::unique_ptr<vk_texture_image>>& getTextureImages() { return _textureImages; };
	const std::vector<std::unique_ptr<vk_texture_image>>& getTextureImages() const { return _textureImages; }
	const vk_texture_image& getTextureImageFromIndex(uint32_t index) const;

	std::vector<std::unique_ptr<vk_graphics_pipeline>>& getGraphicPipelines() { return _graphicsPipelines; };
	const std::vector<std::unique_ptr<vk_graphics_pipeline>>& getGraphicPipelines() const { return _graphicsPipelines; }

	std::vector<std::unique_ptr<vk_mesh>>& getMeshes() { return _meshes; };
	const std::vector<std::unique_ptr<vk_mesh>>& getMeshes() const { return _meshes; }

private:
	struct scene_meshes_info
	{
		uint32_t _totalVertexSize{0};
		uint32_t _totalIndexSize{0};
		std::vector<vk_device_memory::map_memory_region> _vertexMemRegions;
		std::vector<vk_device_memory::map_memory_region> _indexMemRegions;
	};

	void recurseSceneNodes(const gltf::model& m, const gltf::node& selfNode, const mat4& rootMat, scene_meshes_info& info);

	void createMeshesBuffer(scene_meshes_info& info);

	std::vector<std::unique_ptr<vk_texture_image>> _textureImages;
	std::vector<std::unique_ptr<vk_graphics_pipeline>> _graphicsPipelines;
	std::vector<std::unique_ptr<vk_mesh>> _meshes;

	uint32_t _indexVIBufferOffset;
	vk_buffer _meshesVIBuffer;
};
