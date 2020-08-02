#pragma once
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"
#include "vk_buffer.hxx"

#include <vulkan/vulkan.h>
#include <vector>

class vk_device;
class vk_queue_family;
class vk_physical_device;

struct vk_mesh_create_info
{
	const vk_device* device;
	const vk_queue_family* queue_family;
	const vk_physical_device* physical_device;
};

class vk_mesh
{
public:
	vk_mesh();

	void create(const vk_mesh_create_info& create_info);

protected:

	void createVertexBuffer(const vk_device* device, const vk_queue_family* queue_family, const vk_physical_device* physical_device);

	void createIndexBuffer(const vk_device* device, const vk_queue_family* queue_family, const vk_physical_device* physical_device);

	void createUniformBuffers(const vk_device* device, const vk_queue_family* queue_family,
		const vk_physical_device* physical_device, uint32_t imageCount);

private:
	mesh_data _mesh;

	uniforms _ubo;

	vk_buffer _vertex_buffer;

	vk_buffer _index_buffer;

	std::vector<vk_buffer> _uniform_buffers;

	VkRenderPass _vkRenderPass;

	std::vector<VkFramebuffer> _vkFramebuffers;

	VkPipelineLayout _vkPipelineLayout;

	VkPipeline _vkGraphicsPipeline;

	std::vector<VkCommandBuffer> _vkGraphicsCommandBuffers;
};