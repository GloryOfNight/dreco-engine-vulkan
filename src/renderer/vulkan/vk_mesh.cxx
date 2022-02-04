#include "vk_mesh.hxx"

#include "core/utils/file_utils.hxx"
#include "engine/engine.hxx"

#include "vk_queue_family.hxx"
#include "vk_renderer.hxx"
#include "vk_scene.hxx"
#include "vk_utils.hxx"

#include <array>
#include <set>

vk_mesh::vk_mesh()
{
}

vk_mesh::~vk_mesh()
{
	destroy();
}

void vk_mesh::create(const vk_scene& scene, const gltf::mesh& m)
{
	const vk_renderer* renderer = vk_renderer::get();
	const vk::Device device = renderer->getDevice();
	const vk::PhysicalDevice physicalDevice = renderer->getPhysicalDevice();

	const vk_queue_family* queueFamily{&renderer->getQueueFamily()};

	const size_t primitivesSize = m._primitives.size();

	_vertsBufferSize = 0;
	_indxsBufferSize = 0;

	std::set<uint32_t> usedMaterials{};
	std::vector<vk_device_memory::map_memory_region> vertRegions(primitivesSize);
	std::vector<vk_device_memory::map_memory_region> indxRegions(primitivesSize);
	for (const gltf::mesh::primitive& prim : m._primitives)
	{
		const size_t vertBufferSize = sizeof(prim._vertexes[0]) * prim._vertexes.size();
		vertRegions.push_back({prim._vertexes.data(), vertBufferSize, _vertsBufferSize});
		_vertsBufferSize += vertBufferSize;

		const size_t indxBufferSize = sizeof(prim._indexes[0]) * prim._indexes.size();
		indxRegions.push_back({prim._indexes.data(), indxBufferSize, _indxsBufferSize});
		_indxsBufferSize += indxBufferSize;

		usedMaterials.emplace(prim._material);
	}

	for (uint32_t mat : usedMaterials)
	{
		scene.getGraphicPipelines()[mat]->addDependentMesh(this);
	}

	createVIBuffer(m, queueFamily, physicalDevice, vertRegions, indxRegions);
}

void vk_mesh::destroy()
{
	_viBuffer.destroy();
}

void vk_mesh::bindToCmdBuffer(const vk::CommandBuffer commandBuffer) const
{
	const std::array<vk::Buffer, 1> buffers{_viBuffer.get()};
	const std::array<vk::DeviceSize, 1> offsets{0};
	commandBuffer.bindVertexBuffers(0, buffers, offsets);
	commandBuffer.bindIndexBuffer(_viBuffer.get(), _vertsBufferSize, vk::IndexType::eUint32);
	commandBuffer.drawIndexed(static_cast<uint32_t>(_indxsBufferSize / sizeof(uint32_t)), 1, 0, 0, 0);
}

void vk_mesh::update()
{
}

void vk_mesh::createVIBuffer(const gltf::mesh& m, const vk_queue_family* queueFamily, const vk::PhysicalDevice physicalDevice,
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