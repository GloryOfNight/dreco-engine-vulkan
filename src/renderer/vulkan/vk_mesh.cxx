#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_device.hxx"
#include "vk_physical_device.hxx"
#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_scene.hxx"
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

void vk_mesh::create(const mesh& m, const vk_scene* scene)
{
	vk_renderer* renderer{vk_renderer::get()};

	const auto& pipelines = scene->getGraphicPipelines();
	const vk_device* vkDevice{&renderer->getDevice()};
	const vk_queue_family* vkQueueFamily{&renderer->getQueueFamily()};
	const vk_physical_device* vkPhysicalDevice{&renderer->getPhysicalDevice()};

	_vkDevice = vkDevice->get();

	const size_t primitivesSize = m._primitives.size();

	_vertsBufferSize = 0;
	_indxsBufferSize = 0;

	std::vector<vk_graphics_pipeline*> usedPipelines;
	usedPipelines.reserve(primitivesSize);

	std::vector<vk_device_memory::map_memory_region> vertRegions(primitivesSize);
	std::vector<vk_device_memory::map_memory_region> indxRegions(primitivesSize);
	for (const mesh::primitive& prim : m._primitives)
	{
		const size_t vertBufferSize = sizeof(prim._vertexes[0]) * prim._vertexes.size();
		vertRegions.push_back({prim._vertexes.data(), vertBufferSize, _vertsBufferSize});
		_vertsBufferSize += vertBufferSize;

		const size_t indxBufferSize = sizeof(prim._indexes[0]) * prim._indexes.size();
		indxRegions.push_back({prim._indexes.data(), indxBufferSize, _indxsBufferSize});
		_indxsBufferSize += indxBufferSize;

		usedPipelines.emplace_back(pipelines[prim._material]);
	}

	createVIBuffer(m, vkQueueFamily, vkPhysicalDevice, vertRegions, indxRegions);

	_descriptorSet.create(usedPipelines, scene->getTextureImages());
}

void vk_mesh::destroy()
{
	if (VK_NULL_HANDLE != _vkDevice)
	{
		_descriptorSet.destroy();

		_viBuffer.destroy();

		_vkDevice = VK_NULL_HANDLE;
	}
}

void vk_mesh::bindToCmdBuffer(const VkCommandBuffer vkCommandBuffer)
{
	_descriptorSet.bindToCmdBuffer(vkCommandBuffer);

	std::array<VkBuffer, 1> buffers{_viBuffer.get()};
	std::array<VkDeviceSize, 1> offsets{0};
	vkCmdBindVertexBuffers(vkCommandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(vkCommandBuffer, _viBuffer.get(), _vertsBufferSize, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(vkCommandBuffer, static_cast<uint32_t>(_indxsBufferSize / sizeof(uint32_t)), 1, 0, 0, 0);
}

void vk_mesh::update()
{
	const camera* camera{engine::get()->getCamera()};

	_ubo._model = mat4::makeTransform(_transform);
	_ubo._view = camera->getView();
	_ubo._projection = camera->getProjection();

	_descriptorSet.getUniformBuffer().getDeviceMemory().map(&_ubo, sizeof(_ubo));
}

vk_descriptor_set& vk_mesh::getDescriptorSet()
{
	return _descriptorSet;
}

void vk_mesh::createVIBuffer(const mesh& m, const vk_queue_family* queueFamily, const vk_physical_device* physicalDevice,
	const _memory_regions& vertRegions, const _memory_regions& indxRegions)
{
	vk_buffer_create_info buffer_create_info{};
	buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::HOST;
	buffer_create_info.size = _vertsBufferSize + _indxsBufferSize;

	vk_buffer temp_buffer;
	temp_buffer.create(buffer_create_info);

	temp_buffer.getDeviceMemory().map(vertRegions);
	temp_buffer.getDeviceMemory().map(indxRegions, _vertsBufferSize);

	buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_create_info.memory_properties_flags = vk_device_memory_properties::DEVICE_ONLY;

	_viBuffer.create(buffer_create_info);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = buffer_create_info.size;
	vk_buffer::copyBuffer(temp_buffer.get(), _viBuffer.get(), {copyRegion});
}