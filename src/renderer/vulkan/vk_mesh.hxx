#pragma once
#include "math/transform.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"

#include <vector>
#include <vulkan/vulkan.h>

class vk_device;
class vk_queue_family;
class vk_physical_device;

struct vk_mesh_create_info
{
	const vk_device* device;
	const vk_queue_family* queueFamily;
	const vk_physical_device* physicalDevice;
	const VkRenderPass vkRenderPass;
	const VkExtent2D vkExtent;
	const VkCommandBuffer vkCommandBuffer;
	const uint32_t imageCount;
};

class vk_mesh
{
public:
	vk_mesh();
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh();

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create(const vk_mesh_create_info& create_info);

	void recreatePipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

	void destroy();

	void bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer, const uint32_t imageIndex);

	void beforeSubmitUpdate(const uint32_t imageIndex);

protected:
	void createDescriptorPool(const uint32_t imageCount);

	void createDescriptorSetLayot();

	void createDescriptorSets(const uint32_t imageCount);

	void createGraphicsPipelineLayout();

	void createGraphicsPipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

	void createVertexBuffer(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void createIndexBuffer(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void createUniformBuffers(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice, uint32_t imageCount);

	void writeCommandBuffer(const VkRenderPass vkRenderPass);

private:
	transform _transform;

	mesh_data _mesh;

	uniforms _ubo;

	vk_buffer _vertexBuffer;

	vk_buffer _indexBuffer;

	std::vector<vk_buffer> _uniformBuffers;

	VkDevice _vkDevice;

	VkDescriptorPool _vkDescriptorPool;

	VkDescriptorSetLayout _vkDescriptorSetLayout;

	std::vector<VkDescriptorSet> _vkDescriptorSets;

	VkPipelineLayout _vkPipelineLayout;

	VkPipeline _vkGraphicsPipeline;

	VkCommandBuffer _vkCommandBuffer;
};