#pragma once
#include "math/mat4.hxx"

#include <vulkan/vulkan.hpp>

class vk_device;
class vk_scene;
class vk_queue_family;
class vk_graphics_pipeline;
class vk_physical_device;

class vk_mesh final
{
public:
	vk_mesh() = default;
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh() = default;

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void init(uint32_t vertexCount, size_t vertexSize, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset);

	void bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const;

	// temporal hold of the mesh matrix (transform)
	mat4 _mat;

	vk::DeviceSize getVertexSize() const { return _vertexSize; };
	vk::DeviceSize getIndexSize() const { return _indexSize; };

private:
	uint32_t _vertexOffset{0};
	vk::DeviceSize _vertexSize{0};
	vk::DeviceSize _vertexCount{0};

	uint32_t _indexOffset{0};
	vk::DeviceSize _indexSize{0};
	vk::DeviceSize _indexCount{0};
};