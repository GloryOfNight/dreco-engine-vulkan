#include "vk_scene.hxx"

#include "async_tasks/async_load_texture_task.hxx"
#include "core/engine.hxx"

#include "vk_graphics_pipeline.hxx"
#include "vk_mesh.hxx"
#include "vk_texture_image.hxx"

#include <iostream>

vk_scene::~vk_scene()
{
	if (!isEmpty())
	{
		destroy();
	}
}

void vk_scene::create(const gltf::model& m)
{
	if (!isEmpty())
	{
		DE_LOG(Error, "Scene already created! Destroy it before use again.");
		return;
	}
	else if (m._sceneIndex == UINT32_MAX)
	{
		DE_LOG(Error, "Cannot create scene without valid _sceneIndex.");
		return;
	}

	const size_t imagesNum{m._images.size()};
	_textureImages.reserve(imagesNum);
	for (size_t i = 0; i < imagesNum; ++i)
	{
		_textureImages.emplace_back(new vk_texture_image());
		engine::get()->getThreadPool().queueTask(new async_load_texture_task(m._rootPath + '/' + m._images[i]._uri, this, i));
	}

	const size_t totalPipelines = m._materials.size();
	_graphicsPipelines.reserve(totalPipelines);
	for (size_t i = 0; i < totalPipelines; ++i)
	{
		auto& pipeline = _graphicsPipelines.emplace_back(new vk_graphics_pipeline());
		pipeline->create(this, m._materials[i]);
	}

	scene_meshes_info info;

	_meshes.reserve(m._meshes.size());
	const auto& scene = m._scenes[m._sceneIndex];
	for (const auto nodeIndex : scene._nodes)
	{
		recurseSceneNodes(m, m._nodes[nodeIndex], mat4::makeIdentity(), info);
	}

	createMeshesBuffer(info);
}

void vk_scene::recurseSceneNodes(const gltf::model& m, const gltf::node& selfNode, const mat4& rootMat, scene_meshes_info& info)
{
	const mat4 newRootMat = selfNode._matrix * rootMat;
	if (selfNode._mesh != UINT32_MAX)
	{
		const auto& mesh = m._meshes[selfNode._mesh];
		for (const auto& primitive : mesh._primitives)
		{
			auto& newMesh = _meshes.emplace_back(new vk_mesh());

			newMesh->init(primitive._vertexes.size(), info._totalVertexSize / sizeof(gltf::mesh::primitive::vertex), primitive._indexes.size(), info._totalIndexSize / sizeof(uint32_t));
			newMesh->_mat = newRootMat;

			getGraphicPipelines()[primitive._material]->addDependentMesh(newMesh.get());

			const uint32_t vertexSize = newMesh->getVertexSize();
			info._vertexMemRegions.push_back({primitive._vertexes.data(), vertexSize, info._totalVertexSize});
			info._totalVertexSize += vertexSize;

			const uint32_t indexSize = newMesh->getIndexSize();
			info._indexMemRegions.push_back({primitive._indexes.data(), indexSize, info._totalIndexSize});
			info._totalIndexSize += indexSize;
		}
	}
	for (const auto& childNodeIndex : selfNode._children)
	{
		recurseSceneNodes(m, m._nodes[childNodeIndex], newRootMat, info);
	}
}

void vk_scene::createMeshesBuffer(scene_meshes_info& info)
{
	_indexVIBufferOffset = info._totalVertexSize;

	vk_buffer::create_info createInfo;
	createInfo.memoryPropertiesFlags = vk_buffer::create_info::hostMemoryPropertiesFlags;
	createInfo.size = info._totalVertexSize + info._totalIndexSize;
	createInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc;

	vk_buffer tempBuffer;
	tempBuffer.create(createInfo);

	createInfo.memoryPropertiesFlags = vk_buffer::create_info::deviceMemoryPropertiesFlags;
	createInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	_meshesVIBuffer.create(createInfo);

	tempBuffer.getDeviceMemory().map(info._vertexMemRegions);
	tempBuffer.getDeviceMemory().map(info._indexMemRegions, _indexVIBufferOffset);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, createInfo.size);
	vk_buffer::copyBuffer(tempBuffer.get(), _meshesVIBuffer.get(), {copyRegion});
}

void vk_scene::recreatePipelines()
{
	for (auto& pipeline : _graphicsPipelines)
	{
		pipeline->recreatePipeline();
	}
}

void vk_scene::bindToCmdBuffer(vk::CommandBuffer commandBuffer)
{
	vk::Device device = vk_renderer::get()->getDevice();

	std::array<vk::DeviceSize, 1> offsets{0};
	commandBuffer.bindVertexBuffers(0, _meshesVIBuffer.get(), offsets);
	commandBuffer.bindIndexBuffer(_meshesVIBuffer.get(), _indexVIBufferOffset, vk::IndexType::eUint32);

	for (auto& pipeline : _graphicsPipelines)
	{
		pipeline->bindCmd(commandBuffer);
		pipeline->drawCmd(commandBuffer);
	}
}

bool vk_scene::isEmpty() const
{
	return _textureImages.empty() && _graphicsPipelines.empty() && _meshes.empty();
}

void vk_scene::destroy()
{
	_textureImages.clear();
	_graphicsPipelines.clear();
	_meshes.clear();
	_meshesVIBuffer.destroy();
}

const vk_texture_image& vk_scene::getTextureImageFromIndex(uint32_t index) const
{
	const auto& placeholder = vk_renderer::get()->getTextureImagePlaceholder();
	if (index < _textureImages.size() && _textureImages[index]->isValid())
	{
		return *_textureImages[index];
	}
	return placeholder;
}
