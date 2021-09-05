#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_shader_module.hxx"
#include "vk_utils.hxx"

#include <array>

vk_mesh::vk_mesh()
	: _transform{}
	, _vkDevice{VK_NULL_HANDLE}
{
}

vk_mesh::~vk_mesh()
{
	destroy();	
}

void vk_mesh::create(const mesh& m)
{
	vk_renderer* renderer{vk_renderer::get()};

	const vk_device* vkDevice{&renderer->getDevice()};
	const vk_queue_family* vkQueueFamily{&renderer->getQueueFamily()};
	const vk_physical_device* vkPhysicalDevice{&renderer->getPhysicalDevice()};

	_vkDevice = vkDevice->get();

	createVIBuffer(m, vkQueueFamily, vkPhysicalDevice);
	createUniformBuffers(vkQueueFamily, vkPhysicalDevice);

	_textureImage.create();

	createDescriptorSet();
	_graphicsPipeline.create(_descriptorSet);
}

void vk_mesh::recreatePipeline(const VkRenderPass vkRenderPass, const VkExtent2D& vkExtent)
{
	_graphicsPipeline.recreatePipeline();
}

void vk_mesh::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		_descriptorSet.destroy();
		_graphicsPipeline.destroy();

		_viBuffer.destroy();
		_uniformBuffer.destroy();

		_vkDevice = VK_NULL_HANDLE;
	}
}

void vk_mesh::bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer)
{
	vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.get());

	std::array<VkBuffer, 1> buffers{_viBuffer.get()};
	std::array<VkDeviceSize, 1> offsets{0};
	vkCmdBindVertexBuffers(vkCommandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(vkCommandBuffer, _viBuffer.get(), _vertsBufferSize, VK_INDEX_TYPE_UINT32);

	const VkDescriptorSet vkDescriptorSet{_descriptorSet.get()};
	vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.getLayout(), 0, 1, &vkDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(vkCommandBuffer, static_cast<uint32_t>(_indxsBufferSize / sizeof(uint32_t)), 1, 0, 0, 0);
}

void vk_mesh::beforeSubmitUpdate()
{
	const camera* camera{engine::get()->getCamera()};

	_ubo._model = mat4::makeTransform(_transform);
	_ubo._view = camera->getView();
	_ubo._projection = camera->getProjection();

	_uniformBuffer.getDeviceMemory().map(&_ubo, sizeof(_ubo));
}

void vk_mesh::createDescriptorSet()
{
	_descriptorSet.create();

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = _uniformBuffer.get();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(uniforms);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	writeDescriptorSets.resize(2);

	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].pNext = nullptr;
	writeDescriptorSets[0].dstSet = _descriptorSet.get();
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].pBufferInfo = &bufferInfo;
	writeDescriptorSets[0].pImageInfo = nullptr;
	writeDescriptorSets[0].pTexelBufferView = nullptr;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _textureImage.getImageView();
	imageInfo.sampler = _textureImage.getSampler();

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].dstSet = _descriptorSet.get();
	writeDescriptorSets[1].dstBinding = 1;
	writeDescriptorSets[1].dstArrayElement = 0;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].pImageInfo = &imageInfo;

	_descriptorSet.update(writeDescriptorSets);
}

void vk_mesh::createVIBuffer(const mesh& m, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	_vertsBufferSize = 0;
	_indxsBufferSize = 0;

	const size_t primitivesSize = m._primitives.size();
	std::vector<vk_device_memory::map_memory_region> vertsRegions(primitivesSize);
	std::vector<vk_device_memory::map_memory_region> indxsRegions(primitivesSize);
	for (const mesh::primitive& prim : m._primitives)
	{
		const size_t vertBufferSize = sizeof(prim._vertexes[0]) * prim._vertexes.size();
		vertsRegions.push_back({prim._vertexes.data(), vertBufferSize, _vertsBufferSize});
		_vertsBufferSize += vertBufferSize;

		const size_t indxBufferSize = sizeof(prim._indexes[0]) * prim._indexes.size();
		indxsRegions.push_back({prim._indexes.data(), indxBufferSize, _indxsBufferSize});
		_indxsBufferSize += indxBufferSize;
	}

	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::HOST;
	buffer_create_info.size = _vertsBufferSize + _indxsBufferSize;

	vk_buffer temp_buffer;
	temp_buffer.create(buffer_create_info);

	temp_buffer.getDeviceMemory().map(vertsRegions);
	temp_buffer.getDeviceMemory().map(indxsRegions, _vertsBufferSize);
	
	buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::DEVICE_ONLY;

	_viBuffer.create(buffer_create_info);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = buffer_create_info.size;
	vk_buffer::copyBuffer(temp_buffer.get(), _viBuffer.get(), {copyRegion});
}

void vk_mesh::createUniformBuffers(const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::HOST;
	buffer_create_info.size = sizeof(uniforms);

	_uniformBuffer.create(buffer_create_info);
}