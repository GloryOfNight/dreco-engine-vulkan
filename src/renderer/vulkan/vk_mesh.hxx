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
	const VkRenderPass vkRenderPass;
	const VkExtent2D vkExtent;
	const uint32_t imageCount;
};

class vk_mesh
{
public:
	vk_mesh();
	~vk_mesh();

	void create(const vk_mesh_create_info& create_info);

	void destroy();

	void bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer, const uint32_t imageIndex);

	void beforeSubmitUpdate(const uint32_t imageIndex);

protected:

	void createDescriptorPool(const uint32_t imageCount);

	void createDescriptorSetLayot();

	void createDescriptorSets(const uint32_t imageCount);

	void createGraphicsPipelineLayout();

	void createGraphicsPipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

	void createShaderModule(const VkDevice vkDevice, const char* src, const size_t& src_size, VkShaderModule& shaderModule);

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

	VkDevice _vkDevice;

	VkDescriptorPool _vkDescriptorPool;

	VkDescriptorSetLayout _vkDescriptorSetLayout;

	std::vector<VkDescriptorSet> _vkDescriptorSets;

	VkPipelineLayout _vkPipelineLayout;

	VkPipeline _vkGraphicsPipeline;
};