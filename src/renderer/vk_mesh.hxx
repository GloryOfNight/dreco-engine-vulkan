#pragma once
#include "core/containers/gltf/mesh.hxx"
#include "math/transform.hxx"

#include "vk_buffer.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_scene.hxx"
#include "vk_texture_image.hxx"

#include <vector>
#include <vulkan/vulkan.h>

class vk_device;
class vk_scene;
class vk_queue_family;
class vk_graphics_pipeline;
class vk_physical_device;

class vk_mesh final
{
	using _memory_region = vk_device_memory::map_memory_region;

public:
	vk_mesh() = default;
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh() = default;

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create(const vk_scene& scene, const gltf::mesh::primitive& prim, uint32_t vertexOffset, uint32_t indexOffset);

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const;

	uint32_t getVertexesSize() const { return _vertexSize; };
	uint32_t getIndexesSize() const { return _indexSize; };

	mat4 _mat;

private:
	uint32_t _vertexOffset{0};
	vk::DeviceSize _vertexSize{0};
	vk::DeviceSize _vertexCount{0};

	uint32_t _indexOffset{0};
	vk::DeviceSize _indexSize{0};
	vk::DeviceSize _indexCount{0};
};