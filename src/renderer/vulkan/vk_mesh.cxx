#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_scene.hxx"
#include "vk_utils.hxx"

#include <array>
#include <set>

void vk_mesh::create(const vk_scene& scene, const gltf::mesh::primitive& prim, uint32_t vertexOffset, uint32_t indexOffset)
{
	_vertexCount = prim._vertexes.size();
	_vertexSize = _vertexCount * sizeof(gltf::mesh::primitive::vertex);
	_vertexOffset = vertexOffset;

	_indexCount = prim._indexes.size();
	_indexSize = _indexCount * sizeof(uint32_t);
	_indexOffset = indexOffset;

	scene.getGraphicPipelines()[prim._material]->addDependentMesh(this);
}

void vk_mesh::bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const
{
	// draw indexed or draw just verts
	if (_indexCount)
	{
		commandBuffer.drawIndexed(_indexCount, 1, _indexOffset, _vertexOffset, 0);
	}
	else
	{
		commandBuffer.draw(_vertexSize, 1, _vertexOffset, 0);
	}
}