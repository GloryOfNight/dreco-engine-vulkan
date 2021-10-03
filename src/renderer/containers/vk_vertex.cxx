#include "vk_vertex.hxx"

std::vector<VkVertexInputBindingDescription> vk_vertex::getInputBindingDescription()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1, VkVertexInputBindingDescription());

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> vk_vertex::getInputAttributeDescription()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3, VkVertexInputAttributeDescription());

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(vertex, _pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(vertex, _normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(vertex, _texCoord);

	return attributeDescriptions;
}
