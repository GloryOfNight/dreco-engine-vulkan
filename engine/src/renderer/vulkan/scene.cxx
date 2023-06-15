#include "scene.hxx"

#include "core/engine.hxx"
#include "images/texture_image.hxx"
#include "renderer/shader_types/material_data.hxx"

#include "constants.hxx"
#include "material.hxx"
#include "renderer.hxx"
#include "utils.hxx"

#include <iostream>

void de::vulkan::scene::mesh::init(uint32_t vertexCount, size_t vertexSize, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset)
{
	_vertexCount = vertexCount;
	_vertexSize = _vertexCount * vertexSize;
	_vertexOffset = vertexOffset;

	_indexCount = indexCount;
	_indexSize = _indexCount * sizeof(uint32_t);
	_indexOffset = indexOffset;
}

void de::vulkan::scene::mesh::drawCmd(vk::CommandBuffer commandBuffer) const
{
	// draw indexed or draw just verts
	if (_indexCount)
	{
		commandBuffer.drawIndexed(_indexCount, 1, _indexOffset, _vertexOffset, 0);
	}
	else
	{
		commandBuffer.draw(_vertexSize, 1, _vertexOffset, 0);
	}
}

vk::DeviceSize de::vulkan::scene::mesh::getVertexSize() const
{
	return _vertexSize;
}

vk::DeviceSize de::vulkan::scene::mesh::getIndexSize() const
{
	return _indexSize;
}

de::vulkan::scene::~scene()
{
	if (!isEmpty())
	{
		destroy();
	}
}

void de::vulkan::scene::create(const de::gltf::model& m)
{
	auto renderer = renderer::get();
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
		auto& ti = _textureImages.emplace_back(new texture_image());
		ti->create(m._images[i]._image);
	}

	const size_t totalPipelines = m._materials.size();
	std::vector<material_data> materialsData = std::vector<material_data>(totalPipelines, material_data());

	scene_meshes_info info;

	_meshes.resize(totalPipelines);
	const auto& scene = m._scenes[m._sceneIndex];
	for (const auto nodeIndex : scene._nodes)
	{
		recurseSceneNodes(m, m._nodes[nodeIndex], de::math::transform(), info);
	}

	info._materialMemRegions.reserve(totalPipelines);
	for (size_t i = 0; i < totalPipelines; ++i)
	{
		materialsData[i] = material_data(m._materials[i]);

		info._materialMemRegions.emplace_back(device_memory::map_memory_region{&materialsData[i], sizeof(material_data), info._totalMaterialsSize});
		info._totalMaterialsSize += sizeof(materialsData[i]);
	}

	createMeshesBuffer(info);
	createMaterialsBuffer(info);

	const auto basicMat = renderer->getMaterial(de::vulkan::constants::materials::basic);

	for (size_t i = 0; i < totalPipelines; ++i)
	{
		const auto& matData = materialsData[i];

		auto mat = _matInstances.emplace_back(basicMat->makeInstance());

		mat->setBufferDependency("cameraData", &renderer->getCameraDataBuffer());

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

		mat->setBufferDependency("mat", &renderer->getUniformBufferPool().getBuffer(_materialsBufferId));
		mat->updateDescriptorSets();
	}
}

void de::vulkan::scene::recurseSceneNodes(const de::gltf::model& m, const de::gltf::node& selfNode, const de::math::transform& rootTransform, scene_meshes_info& info)
{
	const auto newTransform = selfNode._transform + rootTransform;
	if (selfNode._mesh != UINT32_MAX)
	{
		const auto& mesh = m._meshes[selfNode._mesh];
		for (const auto& primitive : mesh._primitives)
		{
			auto& meshes = _meshes[primitive._material];
			auto& newMesh = meshes.emplace_back(new scene::mesh());

			newMesh->init(primitive._vertexes.size(), sizeof(primitive._vertexes[0]), info._totalVertexSize / sizeof(de::gltf::mesh::primitive::vertex), primitive._indexes.size(), info._totalIndexSize / sizeof(uint32_t));
			newMesh->_mat = de::math::mat4::makeTransform(newTransform);

			const uint32_t vertexSize = newMesh->getVertexSize();
			info._vertexMemRegions.emplace_back(device_memory::map_memory_region{primitive._vertexes.data(), vertexSize, info._totalVertexSize});
			info._totalVertexSize += vertexSize;

			const uint32_t indexSize = newMesh->getIndexSize();
			info._indexMemRegions.emplace_back(device_memory::map_memory_region{primitive._indexes.data(), indexSize, info._totalIndexSize});
			info._totalIndexSize += indexSize;
		}
	}
	for (const auto& childNodeIndex : selfNode._children)
	{
		recurseSceneNodes(m, m._nodes[childNodeIndex], newTransform, info);
	}
}

void de::vulkan::scene::createMeshesBuffer(const scene_meshes_info& info)
{
	_indexOffset = info._totalVertexSize;

	const auto size = info._totalVertexSize + info._totalIndexSize + info._totalMaterialsSize;

	auto renderer = renderer::get();
	auto& bpTransfer = renderer->getTransferBufferPool();
	auto& bpVertIndx = renderer->getVertIndxBufferPool();

	const auto transferBufferId = bpTransfer.makeBuffer(size);
	auto region = bpTransfer.map(transferBufferId);
	for (const auto& reg : info._vertexMemRegions)
	{
		memcpy(reinterpret_cast<uint8_t*>(region) + reg.offset, reg.data, reg.size);
	}
	for (const auto& reg : info._indexMemRegions)
	{
		memcpy(reinterpret_cast<uint8_t*>(region) + reg.offset + _indexOffset, reg.data, reg.size);
	}
	bpTransfer.unmap(transferBufferId);

	_meshesVIBufferId = bpVertIndx.makeBuffer(size);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, size);
	de::vulkan::buffer::copyBuffer(bpTransfer.getBuffer(transferBufferId).get(), bpVertIndx.getBuffer(_meshesVIBufferId).get(), {copyRegion});

	bpTransfer.freeBuffer(transferBufferId);
}

void de::vulkan::scene::createMaterialsBuffer(const scene_meshes_info& info)
{
	auto size = info._totalMaterialsSize;

	auto renderer = renderer::get();
	auto& bpTransfer = renderer->getTransferBufferPool();
	auto& bpUniform = renderer->getUniformBufferPool();

	const auto transferBufferId = bpTransfer.makeBuffer(size);
	auto region = bpTransfer.map(transferBufferId);
	for (const auto& reg : info._materialMemRegions)
	{
		memcpy(reinterpret_cast<uint8_t*>(region) + reg.offset, reg.data, reg.size);
	}
	bpTransfer.unmap(transferBufferId);

	_materialsBufferId = bpUniform.makeBuffer(size);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, size);
	de::vulkan::buffer::copyBuffer(bpTransfer.getBuffer(transferBufferId).get(), bpUniform.getBuffer(_materialsBufferId).get(), {copyRegion});

	bpTransfer.freeBuffer(transferBufferId);
}

void de::vulkan::scene::bindToCmdBuffer(vk::CommandBuffer commandBuffer)
{
	const auto vertIndexBuffer = renderer::get()->getVertIndxBufferPool().getBuffer(_meshesVIBufferId).get();

	std::array<vk::DeviceSize, 1> offsets{0};
	commandBuffer.bindVertexBuffers(0, vertIndexBuffer, offsets);
	commandBuffer.bindIndexBuffer(vertIndexBuffer, _indexOffset, vk::IndexType::eUint32);

	const size_t totalMaterials = _matInstances.size();
	for (size_t i = 0; i < totalMaterials; ++i)
	{
		auto matInst = _matInstances[i];
		auto mat = matInst->getMaterial();

		auto& meshes = _meshes[i];

		mat->bindCmd(commandBuffer);
		matInst->bindCmd(commandBuffer);
		for (auto& mesh : meshes)
		{
			commandBuffer.pushConstants(mat->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(de::math::mat4), &mesh->_mat);
			mesh->drawCmd(commandBuffer);
		}
	}
}

bool de::vulkan::scene::isEmpty() const
{
	return _textureImages.empty() && _meshes.empty();
}

void de::vulkan::scene::destroy()
{
	_textureImages.clear();

	_matInstances.clear();

	_meshes.clear();

	auto renderer = renderer::get();
	renderer->getVertIndxBufferPool().freeBuffer(_meshesVIBufferId);
	renderer->getUniformBufferPool().freeBuffer(_materialsBufferId);
}

const de::vulkan::texture_image& de::vulkan::scene::getTextureImageFromIndex(uint32_t index) const
{
	const auto& placeholder = renderer::get()->getTextureImagePlaceholder();
	if (index < _textureImages.size() && _textureImages[index]->isValid())
	{
		return *_textureImages[index];
	}
	return placeholder;
}
