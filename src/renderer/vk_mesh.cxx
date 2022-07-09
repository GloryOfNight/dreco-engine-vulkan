#include "vk_mesh.hxx"
#include "vk_vertex.hxx"

void vk_mesh::init(uint32_t vertexCount, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset)
{
	_vertexCount = vertexCount;
	_vertexSize = _vertexCount * vk_vertex::size();
	_vertexOffset = vertexOffset;

	_indexCount = indexCount;
	_indexSize = _indexCount * sizeof(uint32_t);
	_indexOffset = indexOffset;
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