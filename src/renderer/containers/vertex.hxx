#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"

#include <vector>
#include <vulkan/vulkan.h>

struct vertex
{
	vec3 _pos;

	vec2 _texCoord;

	static std::vector<VkVertexInputBindingDescription> getInputBindingDescription();

	static std::vector<VkVertexInputAttributeDescription> getInputAttributeDescription();
};