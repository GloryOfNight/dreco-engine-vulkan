#pragma once
#include "core/containers/gltf/model.hxx"
#include "vulkan/vulkan.h"

#include "vk_buffer.hxx"
#include "vk_material.hxx"

#include <memory>
#include <vector>

class vk_texture_image;
class vk_material;
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

private:
	struct scene_meshes_info
	{
		uint32_t _totalVertexSize{0};
		std::vector<vk_device_memory::map_memory_region> _vertexMemRegions;

		uint32_t _totalIndexSize{0};
		std::vector<vk_device_memory::map_memory_region> _indexMemRegions;

		uint32_t _totalMaterialsSize{0};
		std::vector<vk_device_memory::map_memory_region> _materialMemRegions;
	};

	void recurseSceneNodes(const gltf::model& m, const gltf::node& selfNode, const mat4& rootMat, scene_meshes_info& info);

	void createMeshesBuffer(const scene_meshes_info& info);
	void createMaterialsBuffer(const scene_meshes_info& info);

	std::vector<std::unique_ptr<vk_texture_image>> _textureImages;

	vk_material::unique _material;
	std::vector<vk_material_instance*> _matInstances;

	std::vector<std::vector<std::unique_ptr<vk_mesh>>> _meshes;

	uint32_t _indexOffset;
	vk_buffer _meshesVIBuffer;
	vk_buffer _materialsBuffer;
};
