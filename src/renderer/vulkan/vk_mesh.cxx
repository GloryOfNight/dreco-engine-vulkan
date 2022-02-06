#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_scene.hxx"
#include "vk_utils.hxx"

#include <array>
#include <set>

vk_mesh::~vk_mesh()
{
	destroy();
}

void vk_mesh::create(const vk_scene& scene, const gltf::mesh::primitive& prim)
{
	_vertsBufferSize = sizeof(prim._vertexes[0]) * prim._vertexes.size();
	vk_device_memory::map_memory_region vertRegions = {prim._vertexes.data(), _vertsBufferSize, 0};

	_indxsBufferSize = sizeof(uint32_t) * prim._indexes.size();
	vk_device_memory::map_memory_region indxRegions = {prim._indexes.data(), _indxsBufferSize, 0};

	scene.getGraphicPipelines()[prim._material]->addDependentMesh(this);

	createVIBuffer(vertRegions, indxRegions);
}

void vk_mesh::destroy()
{
	_viBuffer.destroy();
}

void vk_mesh::bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const
{
	const std::array<vk::Buffer, 1> buffers{_viBuffer.get()};
	const std::array<vk::DeviceSize, 1> offsets{0};
	commandBuffer.bindVertexBuffers(0, buffers, offsets);

	// draw indexed or draw just verts
	if (_indxsBufferSize)
	{
		commandBuffer.bindIndexBuffer(_viBuffer.get(), _vertsBufferSize, vk::IndexType::eUint32);
		commandBuffer.drawIndexed(static_cast<uint32_t>(_indxsBufferSize / sizeof(uint32_t)), 1, 0, 0, 0);
	}
	else
	{
		commandBuffer.draw(_vertsBufferSize, 1, 0, 0);
	}
}

void vk_mesh::update()
{
}

void vk_mesh::createVIBuffer(const _memory_region& vertRegion, const _memory_region& indxRegion)
{
	vk_buffer::create_info buffer_create_info;
	buffer_create_info.usage =
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc;
	buffer_create_info.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	buffer_create_info.size = _vertsBufferSize + _indxsBufferSize;

	vk_buffer temp_buffer;
	temp_buffer.create(buffer_create_info);

	temp_buffer.getDeviceMemory().map({vertRegion});
	if (indxRegion.size)
		temp_buffer.getDeviceMemory().map({indxRegion}, _vertsBufferSize);

	buffer_create_info.usage =
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	buffer_create_info.memoryPropertiesFlags = vk_buffer::create_info::deviceMemoryPropertiesFlags;

	_viBuffer.create(buffer_create_info);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, buffer_create_info.size);
	vk_buffer::copyBuffer(temp_buffer.get(), _viBuffer.get(), {copyRegion});
}