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
	typedef std::vector<vk_device_memory::map_memory_region> _memory_regions;

public:
	vk_mesh();
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh();

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create(const vk_scene& scene, const gltf::mesh& m);

	void destroy();

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const;

	void update();

	mat4 _mat;

protected:
	void createVIBuffer(const gltf::mesh& m, const vk_queue_family* queueFamily, const vk::PhysicalDevice physicalDevice, const _memory_regions& vertRegions, const _memory_regions& indxRegions);

private:
	vk::DeviceSize _vertsBufferSize{0};
	vk::DeviceSize _indxsBufferSize{0};
	std::vector<uint32_t> _primitiveIndexCounts;

	vk_buffer _viBuffer;
};