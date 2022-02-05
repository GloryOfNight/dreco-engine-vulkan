#pragma once
#include "core/containers/gltf/mesh.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

struct vk_vertex : public gltf::mesh::primitive::vertex
{
	static std::vector<vk::VertexInputBindingDescription> getInputBindingDescription();

	static std::vector<vk::VertexInputAttributeDescription> getInputAttributeDescription();
};