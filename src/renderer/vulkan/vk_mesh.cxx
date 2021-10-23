#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

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
	const vk_renderer* renderer = vk_renderer::get();
	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	const vk_queue_family* queueFamily{&renderer->getQueueFamily()};

	const auto& pipelines = scene->getGraphicPipelines();

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

		usedPipelines.push_back(pipelines[prim._material]);
	}

	createVIBuffer(m, queueFamily, physicalDevice, vertRegions, indxRegions);

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

void vk_mesh::createVIBuffer(const mesh& m, const vk_queue_family* queueFamily, const vk::PhysicalDevice physicalDevice,
	const _memory_regions& vertRegions, const _memory_regions& indxRegions)
{
	vk_buffer::create_info buffer_create_info;
	buffer_create_info.usage = 
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc;
	buffer_create_info.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	buffer_create_info.size = _vertsBufferSize + _indxsBufferSize;

	vk_buffer temp_buffer;
	temp_buffer.create(buffer_create_info);

	temp_buffer.getDeviceMemory().map(vertRegions);
	temp_buffer.getDeviceMemory().map(indxRegions, _vertsBufferSize);

	buffer_create_info.usage = 
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	buffer_create_info.memoryPropertiesFlags = vk_buffer::create_info::deviceMemoryPropertiesFlags;

	_viBuffer.create(buffer_create_info);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, buffer_create_info.size);
	vk_buffer::copyBuffer(temp_buffer.get(), _viBuffer.get(), {copyRegion});
}