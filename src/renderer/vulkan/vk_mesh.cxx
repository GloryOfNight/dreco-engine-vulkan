#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"

#include "vk_allocator.hxx"
#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_shader_module.hxx"
#include "vk_utils.hxx"

#include <array>

vk_mesh::vk_mesh()
	: _vkDevice{VK_NULL_HANDLE}
	, _vkCommandBuffer{VK_NULL_HANDLE}
	, _transform{}
{
}

vk_mesh::~vk_mesh()
{
	destroy();
}

void vk_mesh::create()
{
	vk_renderer* renderer{vk_renderer::get()};

	_mesh = mesh_data::createBox();

	const VkExtent2D currentExtent{renderer->getSurface().getCapabilities().currentExtent};
	const vk_device* vkDevice{&renderer->getDevice()};
	const vk_queue_family* vkQueueFamily{&renderer->getQueueFamily()};
	const vk_physical_device* vkPhysicalDevice{&renderer->getPhysicalDevice()};
	const VkRenderPass vkRenderPass{renderer->getRenderPass()};

	_vkDevice = vkDevice->get();
	_vkCommandBuffer = renderer->createSecondaryCommandBuffer();

	createVertexBuffer(vkDevice, vkQueueFamily, vkPhysicalDevice);
	createIndexBuffer(vkDevice, vkQueueFamily, vkPhysicalDevice);
	createUniformBuffers(vkDevice, vkQueueFamily, vkPhysicalDevice);

	createDescriptorSet();
	_graphicsPipeline.create(_descriptorSet);

	writeCommandBuffer(vkRenderPass);
}

void vk_mesh::recreatePipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent)
{
	_graphicsPipeline.recreatePipeline();
	writeCommandBuffer(vkRenderPass);
}

void vk_mesh::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		_descriptorSet.destroy();
		_graphicsPipeline.destroy();

		_vertexBuffer.destroy();
		_indexBuffer.destroy();
		_uniformBuffer.destroy();

		_vkDevice = VK_NULL_HANDLE;
	}
}

void vk_mesh::bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer, const uint32_t imageIndex)
{
	vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.get());

	std::array<VkBuffer, 1> buffers{_vertexBuffer.get()};
	std::array<VkDeviceSize, 1> offsets{0};
	vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(vkCommandBuffer, _indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);

	const VkDescriptorSet vkDescriptorSet{_descriptorSet.get()};
	vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.getLayout(), 0, 1, &vkDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(vkCommandBuffer, static_cast<uint32_t>(_mesh._indexes.size()), 1, 0, 0, 0);
}

void vk_mesh::beforeSubmitUpdate(const uint32_t imageIndex)
{
	_ubo._model = mat4::makeTransform(_transform);
	_ubo._view = mat4::makeTranslation(vec3{0, 0, 1.3F});
	_ubo._projection = mat4::makeProjection(-1, 1, static_cast<float>(800) / static_cast<float>(800), 75.F);

	_uniformBuffer.map(&_ubo, sizeof(_ubo));
}

void vk_mesh::createDescriptorSet()
{
	_descriptorSet.create();

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = _uniformBuffer.get();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(uniforms);

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext = nullptr;
	descriptorWrite.dstSet = _descriptorSet.get();
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	std::vector<VkWriteDescriptorSet> writeInfo{descriptorWrite};
	_descriptorSet.update(writeInfo);
}

void vk_mesh::createGraphicsPipeline()
{
	
}

void vk_mesh::createVertexBuffer(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::VERTEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(_mesh._vertexes[0]) * _mesh._vertexes.size();

	_vertexBuffer.create(device, buffer_create_info);
	_vertexBuffer.map(_mesh._vertexes.data(), buffer_create_info.size);
}

void vk_mesh::createIndexBuffer(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::INDEX;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(_mesh._indexes[0]) * _mesh._indexes.size();

	_indexBuffer.create(device, buffer_create_info);
	_indexBuffer.map(_mesh._indexes.data(), buffer_create_info.size);
}

void vk_mesh::createUniformBuffers(const vk_device* device, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = vk_buffer_usage::UNIFORM;
	buffer_create_info.memory_properties = vk_buffer_memory_properties::DEVICE;
	buffer_create_info.queueFamily = queueFamily;
	buffer_create_info.physicalDevice = physicalDevice;
	buffer_create_info.size = sizeof(uniforms);

	_uniformBuffer.create(device, buffer_create_info);
}

void vk_mesh::writeCommandBuffer(const VkRenderPass vkRenderPass)
{
	VkCommandBufferInheritanceInfo inheritanceInfo{};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.pNext = nullptr;
	inheritanceInfo.renderPass = vkRenderPass;
	inheritanceInfo.framebuffer = VK_NULL_HANDLE;
	inheritanceInfo.subpass = 0;
	inheritanceInfo.pipelineStatistics = 0;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	VK_CHECK(vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo));
	bindToCmdBuffer(_vkCommandBuffer, 0);
	VK_CHECK(vkEndCommandBuffer(_vkCommandBuffer));
}