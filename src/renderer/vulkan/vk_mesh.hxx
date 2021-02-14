#pragma once
#include "math/transform.hxx"
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

#include "vk_buffer.hxx"
#include "vk_descriptor_set.hxx"
#include "vk_graphics_pipeline.hxx"
#include "vk_texture_image.hxx"

#include <vector>
#include <vulkan/vulkan.h>

class vk_device;
class vk_queue_family;
class vk_graphics_pipeline;
class vk_physical_device;

class vk_mesh final
{
public:
	vk_mesh(const mesh_data& meshData);
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh();

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create();

	void recreatePipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

	void destroy();

	void bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer, const uint32_t imageIndex);

	void beforeSubmitUpdate(const uint32_t imageIndex);

	transform _transform;

protected:
	void createDescriptorSet();

	void createGraphicsPipeline();

	void createVertexBuffer(const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void createIndexBuffer(const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void createUniformBuffers(const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void writeCommandBuffer(const VkRenderPass vkRenderPass);

private:
	mesh_data _mesh;

	uniforms _ubo;

	vk_buffer _vertexBuffer;

	vk_buffer _indexBuffer;

	vk_buffer _uniformBuffer;

	vk_texture_image _textureImage;

	vk_graphics_pipeline _graphicsPipeline;

	vk_descriptor_set _descriptorSet;

	VkDevice _vkDevice;

	VkCommandBuffer _vkCommandBuffer;
};