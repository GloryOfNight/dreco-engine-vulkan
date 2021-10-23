#include "vk_vertex.hxx"

std::vector<vk::VertexInputBindingDescription> vk_vertex::getInputBindingDescription()
{
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1, VkVertexInputBindingDescription());

	bindingDescriptions[0] =
		vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(vertex))
			.setInputRate(vk::VertexInputRate::eVertex);

	return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> vk_vertex::getInputAttributeDescription()
{
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3, VkVertexInputAttributeDescription());

	attributeDescriptions[0] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(vertex, _pos));

	attributeDescriptions[1] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(vertex, _normal));

	attributeDescriptions[2] =
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(2)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(vertex, _texCoord));

	return attributeDescriptions;
}
