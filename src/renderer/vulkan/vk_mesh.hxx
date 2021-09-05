#pragma once
#include "math/transform.hxx"
#include "core/containers/mesh.hxx"
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
	vk_mesh();
	vk_mesh(const vk_mesh&) = delete;
	vk_mesh(vk_mesh&&) = delete;
	~vk_mesh();

	vk_mesh& operator=(const vk_mesh&) = delete;
	vk_mesh& operator=(vk_mesh&&) = delete;

	void create(const mesh& m);

	void recreatePipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent);

	void destroy();

	void bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer);

	void beforeSubmitUpdate();

	transform _transform;

protected:
	void createDescriptorSet();

	void createVIBuffer(const mesh& m, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

	void createUniformBuffers(const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice);

private:
	VkDeviceSize _vertsBufferSize{0};
	VkDeviceSize _indxsBufferSize{0};

	uniforms _ubo;

	vk_buffer _viBuffer;

	vk_buffer _uniformBuffer;

	vk_texture_image _textureImage;

	vk_graphics_pipeline _graphicsPipeline;

	vk_descriptor_set _descriptorSet;

	VkDevice _vkDevice;
};