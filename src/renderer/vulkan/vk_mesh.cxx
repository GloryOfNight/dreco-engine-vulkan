#include "vk_mesh.hxx"
#include "vk_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_physical_device.hxx"

vk_mesh::vk_mesh()
{
}

void vk_mesh::create(const vk_mesh_create_info& create_info)
{
}

void vk_mesh::createVertexBuffer(const vk_device* device, const vk_queue_family* queue_family, const vk_physical_device* physical_device)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::VERTEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queue_family = queue_family;
	buffer_create_info.physical_device = physical_device;
	buffer_create_info.size = sizeof(_mesh._vertexes[0]) * _mesh._vertexes.size();

	_vertex_buffer.create(device, buffer_create_info);
	_vertex_buffer.map(_mesh._vertexes.data(), buffer_create_info.size);
}

void vk_mesh::createIndexBuffer(const vk_device* device, const vk_queue_family* queue_family, const vk_physical_device* physical_device)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::INDEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queue_family = queue_family;
	buffer_create_info.physical_device = physical_device;
	buffer_create_info.size = sizeof(_mesh._indexes[0]) * _mesh._indexes.size();

	_index_buffer.create(device, buffer_create_info);
	_index_buffer.map(_mesh._indexes.data(), buffer_create_info.size);
}

void vk_mesh::createUniformBuffers(const vk_device* device, const vk_queue_family* queue_family,
	const vk_physical_device* physical_device, uint32_t imageCount)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::UNIFORM;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queue_family = queue_family;
	buffer_create_info.physical_device = physical_device;
	buffer_create_info.size = sizeof(uniforms);

	_uniform_buffers.resize(imageCount);
	for (auto& buffer : _uniform_buffers)
	{
		buffer.create(device, buffer_create_info);
	}
}
