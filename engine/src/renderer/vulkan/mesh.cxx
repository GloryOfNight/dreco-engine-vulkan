#include "mesh.hxx"

void de::vulkan::mesh::init(uint32_t vertexCount, size_t vertexSize, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset)
{
	_vertexCount = vertexCount;
	_vertexSize = _vertexCount * vertexSize;
	_vertexOffset = vertexOffset;

	_indexCount = indexCount;
	_indexSize = _indexCount * sizeof(uint32_t);
	_indexOffset = indexOffset;
}

void de::vulkan::mesh::drawCmd(vk::CommandBuffer commandBuffer) const
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

vk::DeviceSize de::vulkan::mesh::getVertexSize() const
{
	return _vertexSize;
}

vk::DeviceSize de::vulkan::mesh::getIndexSize() const
{
	return _indexSize;
}