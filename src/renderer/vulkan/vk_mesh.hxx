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
	~vk_mesh();

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create(const vk_scene& scene, const gltf::mesh::primitive& prim);

	void destroy();

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const;

	void update();

	mat4 _mat;

protected:
	void createVIBuffer(const _memory_region& vertRegion, const _memory_region& indxRegion);

private:
	vk::DeviceSize _vertsBufferSize{0};
	vk::DeviceSize _indxsBufferSize{0};
	std::vector<uint32_t> _primitiveIndexCounts;

	vk_buffer _viBuffer;
};