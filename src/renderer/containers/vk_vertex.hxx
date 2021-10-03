#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"
#include "core/containers/mesh.hxx"

#include <vector>
#include <vulkan/vulkan.h>

struct vk_vertex : public mesh::primitive::vertex
{
	static std::vector<VkVertexInputBindingDescription> getInputBindingDescription();

	static std::vector<VkVertexInputAttributeDescription> getInputAttributeDescription();
};