#include "vk_scene.hxx"

#include "core/engine.hxx"
#include "images/vk_texture_image.hxx"
#include "shader_types/material_data.hxx"

#include "vk_graphics_pipeline.hxx"
#include "vk_material.hxx"
#include "vk_mesh.hxx"
#include "vk_utils.hxx"

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
	auto renderer = vk_renderer::get();
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
		auto& ti = _textureImages.emplace_back(new vk_texture_image());
		ti->create(m._images[i]._image);
	}

	const size_t totalPipelines = m._materials.size();
	std::vector<material_data> materialsData = std::vector<material_data>(totalPipelines, material_data());

	scene_meshes_info info;

	_meshes.resize(totalPipelines);
	const auto& scene = m._scenes[m._sceneIndex];
	for (const auto nodeIndex : scene._nodes)
	{
		recurseSceneNodes(m, m._nodes[nodeIndex], mat4::makeIdentity(), info);
	}

	info._materialMemRegions.reserve(totalPipelines);
	for (size_t i = 0; i < totalPipelines; ++i)
	{
		materialsData[i] = material_data(m._materials[i]);

		info._materialMemRegions.emplace_back(vk_device_memory::map_memory_region{&materialsData[i], sizeof(material_data), info._totalMaterialsSize});
		info._totalMaterialsSize += sizeof(materialsData[i]);
	}

	createMeshesBuffer(info);
	createMaterialsBuffer(info);

	auto vert = renderer->loadShader(DRECO_SHADER("basic.vert.spv"));
	auto frag = renderer->loadShader(DRECO_SHADER("basic.frag.spv"));
	_material = vk_material::makeNew(vert, frag, totalPipelines);

	for (size_t i = 0; i < totalPipelines; ++i)
	{
		const auto& matData = materialsData[i];

		auto& mat = _matInstances.emplace_back(&_material->makeInstance());

		mat->setBufferDependency("cameraData", renderer->getCameraDataBuffer());

		const auto baseColorIndex = m._materials[i]._pbrMetallicRoughness._baseColorTexture._index;
		const auto metallicRoughnessIndex = m._materials[i]._pbrMetallicRoughness._metallicRoughnessTexture._index;
		const auto emissiveIndex = m._materials[i]._emissive._index;
		const auto normalIndex = m._materials[i]._normal._index;

		if (matData._hasBaseColor)
			mat->setImageDependecy("baseColor", _textureImages[baseColorIndex].get());
		if (matData._hasMetallicRoughness)
			mat->setImageDependecy("metallicRoughness", _textureImages[metallicRoughnessIndex].get());
		if (matData._hasEmissive)
			mat->setImageDependecy("emissive", _textureImages[emissiveIndex].get());
		if (matData._hasNormal)
			mat->setImageDependecy("normal", _textureImages[normalIndex].get());

		mat->setBufferDependency("mat", _materialsBuffer);
		mat->updateDescriptorSets();
	}
}

void vk_scene::recurseSceneNodes(const gltf::model& m, const gltf::node& selfNode, const mat4& rootMat, scene_meshes_info& info)
{
	const mat4 newRootMat = selfNode._matrix * rootMat;
	if (selfNode._mesh != UINT32_MAX)
	{
		const auto& mesh = m._meshes[selfNode._mesh];
		for (const auto& primitive : mesh._primitives)
		{
			auto& meshes = _meshes[primitive._material];
			auto& newMesh = meshes.emplace_back(new vk_mesh());

			newMesh->init(primitive._vertexes.size(), sizeof(primitive._vertexes[0]), info._totalVertexSize / sizeof(gltf::mesh::primitive::vertex), primitive._indexes.size(), info._totalIndexSize / sizeof(uint32_t));
			newMesh->_mat = newRootMat;

			const uint32_t vertexSize = newMesh->getVertexSize();
			info._vertexMemRegions.emplace_back(vk_device_memory::map_memory_region{primitive._vertexes.data(), vertexSize, info._totalVertexSize});
			info._totalVertexSize += vertexSize;

			const uint32_t indexSize = newMesh->getIndexSize();
			info._indexMemRegions.emplace_back(vk_device_memory::map_memory_region{primitive._indexes.data(), indexSize, info._totalIndexSize});
			info._totalIndexSize += indexSize;
		}
	}
	for (const auto& childNodeIndex : selfNode._children)
	{
		recurseSceneNodes(m, m._nodes[childNodeIndex], newRootMat, info);
	}
}

void vk_scene::createMeshesBuffer(const scene_meshes_info& info)
{
	_indexOffset = info._totalVertexSize;

	auto size = info._totalVertexSize + info._totalIndexSize + info._totalMaterialsSize;
	auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc;

	vk_buffer tempBuffer;
	tempBuffer.allocate(vk_utils::memory_property::host, usage, size);
	
	usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	_meshesVIBuffer.allocate(vk_utils::memory_property::device, usage, size);

	tempBuffer.getDeviceMemory().map(info._vertexMemRegions);
	tempBuffer.getDeviceMemory().map(info._indexMemRegions, _indexOffset);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, size);
	vk_buffer::copyBuffer(tempBuffer.get(), _meshesVIBuffer.get(), {copyRegion});
}

void vk_scene::createMaterialsBuffer(const scene_meshes_info& info)
{
	auto size = info._totalMaterialsSize;
	auto usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc;

	vk_buffer tempBuffer;
	tempBuffer.allocate(vk_utils::memory_property::host, usage, size);

	usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
	_materialsBuffer.allocate(vk_utils::memory_property::device, usage, size);

	tempBuffer.getDeviceMemory().map(info._materialMemRegions);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, size);
	vk_buffer::copyBuffer(tempBuffer.get(), _materialsBuffer.get(), {copyRegion});
}

void vk_scene::recreatePipelines()
{
	_material->recreatePipeline();
}

void vk_scene::bindToCmdBuffer(vk::CommandBuffer commandBuffer)
{
	std::array<vk::DeviceSize, 1> offsets{0};
	commandBuffer.bindVertexBuffers(0, _meshesVIBuffer.get(), offsets);
	commandBuffer.bindIndexBuffer(_meshesVIBuffer.get(), _indexOffset, vk::IndexType::eUint32);

	const size_t totalMaterials = _matInstances.size();
	for (size_t i = 0; i < totalMaterials; ++i)
	{
		auto& mat = _matInstances[i];
		auto& meshes = _meshes[i];

		mat->bindCmd(commandBuffer);
		for (auto& mesh : meshes)
		{
			commandBuffer.pushConstants(mat->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4), &mesh->_mat);
			mesh->bindToCmdBuffer(commandBuffer);
		}
	}
}

bool vk_scene::isEmpty() const
{
	return _textureImages.empty() && _meshes.empty();
}

void vk_scene::destroy()
{
	_textureImages.clear();

	_material.reset();
	_matInstances.clear();

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
