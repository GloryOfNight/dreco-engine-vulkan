#include "gltf_loader.hxx"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "core/containers/gltf/model.hxx"
#include "core/utils/log.hxx"
#include "math/rotator.hxx"
#include "tinygltf/tiny_gltf.h"

#include <filesystem>
#include <iostream>
#include <vector>

// t - for tiny, d - for dreco

static mat4 parseMatrix(const std::vector<double>& matrix)
{
	mat4 out = mat4::makeIdentity();

	if (matrix.size() == 16)
	{
		for (uint8_t i = 0; i < 4; ++i)
		{
			const uint8_t indx = i * 4;
			out._mat[i][0] = matrix[indx];
			out._mat[i][1] = matrix[indx + 1];
			out._mat[i][2] = matrix[indx + 2];
			out._mat[i][3] = matrix[indx + 3];
		}
	}
	return out;
}

static void parseScenes(const tinygltf::Model& tModel, gltf::model& dModel)
{
	dModel._sceneIndex = static_cast<uint32_t>(tModel.defaultScene);

	const size_t totalScenes = tModel.scenes.size();
	dModel._scenes.resize(totalScenes);
	for (size_t i = 0; i < totalScenes; ++i)
	{
		const auto& tScene = tModel.scenes[i];
		auto& dScene = dModel._scenes[i];

		const size_t sceneTotalNodes = tScene.nodes.size();
		dScene._nodes.resize(sceneTotalNodes);
		for (size_t k = 0; k < sceneTotalNodes; ++k)
		{
			dScene._nodes[k] = static_cast<uint32_t>(tScene.nodes[k]);
		}
	}
}

static void parseNodes(const tinygltf::Model& tModel, gltf::model& dModel)
{
	const size_t totalNodes = tModel.nodes.size();
	dModel._nodes.resize(totalNodes);
	for (size_t i = 0; i < totalNodes; ++i)
	{
		const auto& tNode = tModel.nodes[i];
		auto& dNode = dModel._nodes[i];

		const size_t totalChildren = tNode.children.size();
		dNode._children.resize(totalChildren);
		for (size_t i = 0; i < totalChildren; ++i)
		{
			dNode._children[i] = static_cast<uint32_t>(tNode.children[i]);
		}

		dNode._mesh = static_cast<uint32_t>(tNode.mesh);

		dNode._matrix = mat4::makeIdentity();
		if (tNode.matrix.empty())
		{
			if (tNode.translation.size() == 3)
			{
				const vec3 translation = vec3(tNode.translation[0], tNode.translation[1], tNode.translation[2]);
				dNode._matrix = dNode._matrix * mat4::makeTranslation(translation);
			}
			if (tNode.rotation.size() == 4)
			{
				const quaternion quat = quaternion{tNode.rotation[0], tNode.rotation[1], tNode.rotation[2], tNode.rotation[3]};
				dNode._matrix = dNode._matrix * mat4::makeRotationQ(quat);
			}
			if (tNode.scale.size() == 3)
			{
				const vec3 scale = vec3(tNode.scale[0], tNode.scale[1], tNode.scale[2]);
				dNode._matrix = dNode._matrix * mat4::makeScale(scale);
			}
		}
		else
		{
			dNode._matrix = parseMatrix(tNode.matrix);
		}
	}
}

static void parseMeshes(const tinygltf::Model& tModel, gltf::model& dModel)
{
	const size_t totalMeshes = tModel.meshes.size();
	dModel._meshes.resize(totalMeshes);
	for (size_t i = 0; i < totalMeshes; ++i)
	{
		const auto& tMesh = tModel.meshes[i];
		auto& dMesh = dModel._meshes[i];

		const size_t totalMeshPrimites = tMesh.primitives.size();
		dMesh._primitives.resize(totalMeshPrimites);
		for (size_t k = 0; k < totalMeshPrimites; ++k)
		{
			const auto& tPrimitive = tMesh.primitives[k];
			auto& dPrimitive = dMesh._primitives[k];

			dPrimitive._material = static_cast<uint32_t>(tPrimitive.material);

			uint32_t vertPosAccessor{UINT32_MAX};
			uint32_t normalAccessor{UINT32_MAX};
			uint32_t texCoordAccessor{UINT32_MAX};
			uint32_t indexAccessor{static_cast<uint32_t>(tPrimitive.indices)};
			for (const auto& attr : tPrimitive.attributes)
			{
				if (attr.first == "POSITION")
				{
					vertPosAccessor = attr.second;
				}
				else if (attr.first == "NORMAL")
				{
					normalAccessor = attr.second;
				}
				else if (attr.first == "TEXCOORD_0")
				{
					texCoordAccessor = attr.second;
				}
			}

			dPrimitive._vertexes.resize(tModel.accessors[vertPosAccessor].count);
			dPrimitive._indexes.resize(tModel.accessors[indexAccessor].count);

			const std::array<uint32_t, 4> usedAccessors{vertPosAccessor, normalAccessor, texCoordAccessor, indexAccessor};
			for (const uint32_t accessorIndex : usedAccessors)
			{
				const auto& accessor{tModel.accessors[accessorIndex]};
				const auto& bufferView{tModel.bufferViews[accessor.bufferView]};
				const auto& buffer{tModel.buffers[bufferView.buffer]};
				const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t q = 0; q < accessor.count; ++q)
				{
					if (accessorIndex == vertPosAccessor && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						vec3& pos{dPrimitive._vertexes[q]._pos};
						pos._x = positions[q * 3 + 0];
						pos._y = positions[q * 3 + 1];
						pos._z = positions[q * 3 + 2];
					}
					else if (accessorIndex == indexAccessor)
					{
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						{
							const uint16_t* indexPos = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
							dPrimitive._indexes[q] = indexPos[q];
						}
						else
						{
							const uint32_t* indexPos = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
							dPrimitive._indexes[q] = indexPos[q];
						}
					}
					else if (accessorIndex == texCoordAccessor && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						vec2& texCoor{dPrimitive._vertexes[q]._texCoord};
						texCoor._x = positions[q * 2 + 0];
						texCoor._y = positions[q * 2 + 1];
					}
					else if (accessorIndex == normalAccessor && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						vec3& normal{dPrimitive._vertexes[q]._normal};
						normal._x = positions[q * 3 + 0];
						normal._y = positions[q * 3 + 1];
						normal._z = positions[q * 3 + 2];
					}
				}
			}
		}
	}
}

static void parseMaterials(const tinygltf::Model& tModel, gltf::model& dModel)
{
	const size_t totalMaterials = tModel.materials.size();
	dModel._materials.resize(totalMaterials);
	for (size_t i = 0; i < totalMaterials; ++i)
	{
		const auto& tMat = tModel.materials[i];
		auto& dMat = dModel._materials[i];

		dMat._doubleSided = tMat.doubleSided;
		dMat._normalTexture._index = static_cast<uint32_t>(tMat.normalTexture.index);
		dMat._normalTexture._scale = tMat.normalTexture.scale;

		dMat._emissiveTexture._index = static_cast<uint32_t>(tMat.emissiveTexture.index);

		dMat._occlusionTexture._index = static_cast<uint32_t>(tMat.occlusionTexture.index);
		dMat._occlusionTexture._strength = tMat.occlusionTexture.strength;

		std::memcpy(dMat._pbrMetallicRoughness._baseColorFactor.data(), tMat.pbrMetallicRoughness.baseColorFactor.data(), sizeof(double) * 4);
		dMat._pbrMetallicRoughness._baseColorTexture._index = static_cast<uint32_t>(tMat.pbrMetallicRoughness.baseColorTexture.index);
		dMat._pbrMetallicRoughness._metallicFactor = tMat.pbrMetallicRoughness.metallicFactor;
		dMat._pbrMetallicRoughness._metallicRoughnessTexture._index = static_cast<uint32_t>(tMat.pbrMetallicRoughness.metallicRoughnessTexture.index);
		dMat._pbrMetallicRoughness._roughnessFactor = tMat.pbrMetallicRoughness.roughnessFactor;
	}
}

static void parseImages(const tinygltf::Model& tModel, gltf::model& dModel)
{
	const size_t totalImages = tModel.images.size();
	dModel._images.resize(totalImages);
	for (size_t i = 0; i < totalImages; ++i)
	{
		dModel._images[i]._uri = tModel.images[i].uri;
	}
}

gltf::model gltf_loader::loadModel(const std::string_view& sceneFile)
{
	tinygltf::Model tModel;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	const bool result = loader.LoadASCIIFromFile(&tModel, &err, &warn, sceneFile.data());
	if (!result)
	{
		DE_LOG(Error, "Failed to load scene: %s; Current work dir: %s", sceneFile.data(), std::filesystem::current_path().generic_string().data());
		return {};
	}
	else if (!warn.empty())
	{
		DE_LOG(Warn, "Load scene warning: %s", warn.data());
	}

	gltf::model dModel;
	dModel._rootPath = std::filesystem::path(sceneFile).parent_path().generic_string();
	parseScenes(tModel, dModel);
	parseNodes(tModel, dModel);
	parseMeshes(tModel, dModel);
	parseMaterials(tModel, dModel);
	parseImages(tModel, dModel);

	return dModel;
}