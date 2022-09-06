#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

struct vk_vertex
{
	static vk::DeviceSize size();
	
	static std::vector<vk::VertexInputBindingDescription> getInputBindingDescription();

	static std::vector<vk::VertexInputAttributeDescription> getInputAttributeDescription();
};