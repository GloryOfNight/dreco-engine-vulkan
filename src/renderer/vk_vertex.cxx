#include "vk_vertex.hxx"

#include "core/containers/gltf/mesh.hxx"

vk::DeviceSize vk_vertex::size()
{
	return sizeof(gltf::mesh::primitive::vertex);
}

std::vector<vk::VertexInputBindingDescription> vk_vertex::getInputBindingDescription()
{
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1, VkVertexInputBindingDescription());

	bindingDescriptions[0] =
		vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(size())
			.setInputRate(vk::VertexInputRate::eVertex);

	return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> vk_vertex::getInputAttributeDescription()
{
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(4, VkVertexInputAttributeDescription());

	attributeDescriptions[0] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(gltf::mesh::primitive::vertex, _pos));

	attributeDescriptions[1] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(gltf::mesh::primitive::vertex, _normal));

	attributeDescriptions[2] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(2)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(gltf::mesh::primitive::vertex, _texCoord));

		attributeDescriptions[3] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(3)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(offsetof(gltf::mesh::primitive::vertex, _color));

	return attributeDescriptions;
}
