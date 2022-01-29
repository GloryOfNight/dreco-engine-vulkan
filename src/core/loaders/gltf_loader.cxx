#include "gltf_loader.hxx"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "core/utils/log.hxx"
#include "math/rotator.hxx"
#include "tinygltf/tiny_gltf.h"

#include <filesystem>
#include <iostream>
#include <vector>

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

static void parseNodeRecurse(const tinygltf::Model& model, const tinygltf::Node& self, const mat4& rootMat, scene& outScene)
{
	mat4 selfMatrix = parseMatrix(self.matrix) * rootMat;
	for (const int childNodeIndex : self.children)
	{
		const auto& childNode = model.nodes[childNodeIndex];
		parseNodeRecurse(model, model.nodes[childNodeIndex], selfMatrix, outScene);
	}

	if (self.mesh >= 0)
	{
		const auto& modelMesh = model.meshes[self.mesh];

		const size_t totalMeshPrimites = modelMesh.primitives.size();

		auto& sceneMesh = outScene._meshes[self.mesh];
		sceneMesh._matrix = selfMatrix;

		sceneMesh._primitives.resize(totalMeshPrimites);

		for (size_t k = 0; k < totalMeshPrimites; ++k)
		{
			auto& sceneMeshPrimitive = sceneMesh._primitives[k];
			const auto& primitive = modelMesh.primitives[k];

			sceneMeshPrimitive._material = primitive.material;

			uint32_t vertPosAccessor{UINT32_MAX};
			uint32_t normalAccessor{UINT32_MAX};
			uint32_t texCoordAccessor{UINT32_MAX};
			uint32_t indexAccessor{static_cast<uint32_t>(primitive.indices)};
			for (const auto& attr : primitive.attributes)
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

			sceneMeshPrimitive._vertexes.resize(model.accessors[vertPosAccessor].count);
			sceneMeshPrimitive._indexes.resize(model.accessors[indexAccessor].count);

			const size_t accessorsSize{model.accessors.size()};
			for (size_t j = 0; j < accessorsSize; ++j)
			{
				const auto& accessor{model.accessors[j]};
				const auto& bufferView{model.bufferViews[accessor.bufferView]};
				const auto& buffer{model.buffers[bufferView.buffer]};

				const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t q = 0; q < accessor.count; ++q)
				{
					if (j == vertPosAccessor)
					{
						vec3& pos{sceneMeshPrimitive._vertexes[q]._pos};
						pos._x = positions[q * 3 + 0];
						pos._y = positions[q * 3 + 1];
						pos._z = positions[q * 3 + 2];
					}
					else if (j == indexAccessor)
					{
						const uint32_t* indexPos = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						sceneMeshPrimitive._indexes[q] = indexPos[q];
					}
					else if (j == texCoordAccessor)
					{
						vec2& texCoor{sceneMeshPrimitive._vertexes[q]._texCoord};
						texCoor._x = positions[q * 2 + 0];
						texCoor._y = positions[q * 2 + 1];
					}
					else if (j == normalAccessor)
					{
						vec3& normal{sceneMeshPrimitive._vertexes[q]._normal};
						normal._x = positions[q * 3 + 0];
						normal._y = positions[q * 3 + 1];
						normal._z = positions[q * 3 + 2];
					}
				}
			}
		}
	}
}

scene gltf_loader::loadScene(const std::string_view& sceneFile)
{
	using namespace tinygltf;

	Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	const bool result = loader.LoadASCIIFromFile(&model, &err, &warn, sceneFile.data());
	if (!result)
	{
		DE_LOG(Error, "Failed to load scene: %s; Current work dir: %s", sceneFile.data(), std::filesystem::current_path().generic_string().data());
		return {};
	}
	else if (!warn.empty())
	{
		DE_LOG(Warn, "Load scene warning: %s", warn.data());
	}

	const size_t totalNodes = model.nodes.size();
	const size_t totalMeshes = model.meshes.size();
	const size_t totalImages = model.images.size();
	const size_t totalMaterials = model.materials.size();

	scene newScene{};
	newScene._meshes.resize(totalMeshes);
	newScene._images.resize(totalImages);
	newScene._materials.resize(totalMaterials);

	for (const auto& modelScene : model.scenes)
	{
		for (const int sceneNodeIndex : modelScene.nodes)
		{
			const auto& sceneNode = model.nodes[sceneNodeIndex];

			mat4 rot = mat4::makeRotation(rotator(90, 0, 0));
			mat4 rootNodeMatrix = rot * parseMatrix(sceneNode.matrix);
			parseNodeRecurse(model, sceneNode, rootNodeMatrix, newScene);
		}
	}

	const std::string parentPath = std::filesystem::path(sceneFile).parent_path().generic_string();
	for (size_t i = 0; i < totalImages; ++i)
	{
		newScene._images[i]._uri = parentPath + "/" + model.images[i].uri;
	}

	for (size_t i = 0; i < totalMaterials; ++i)
	{
		newScene._materials[i]._doubleSided = model.materials[i].doubleSided;
		auto index = model.materials[i].pbrMetallicRoughness.baseColorTexture.index;
		if (index >= 0)
		{
			newScene._materials[i]._baseColorTexture = index;
		}
		else
		{
			// should not do this, but not yet supported untextured stuff
			newScene._materials[i]._baseColorTexture = 0;
		}
	}

	return newScene;
}