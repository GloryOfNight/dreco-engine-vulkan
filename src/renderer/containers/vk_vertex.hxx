#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"
#include "core/containers/mesh.hxx"

#include <vector>
#include <vulkan/vulkan.hpp>

struct vk_vertex : public mesh::primitive::vertex
{
	static std::vector<vk::VertexInputBindingDescription> getInputBindingDescription();

	static std::vector<vk::VertexInputAttributeDescription> getInputAttributeDescription();
};