#include "gltf_loader.hxx"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tinygltf/tiny_gltf.h"

#include <filesystem>
#include <iostream>
#include <vector>

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
		std::cerr << __FUNCTION__ << ": " << "Err: " << err;
		std::cout << __FUNCTION__ << ": " << "Verb: " << "Current working directory: " << std::filesystem::current_path() << std::endl;
		return {};
	}
	else if (!warn.empty())
	{
		std::cout << __FUNCTION__ << ": " << "Warn: " << warn << std::endl;
	}

	const size_t totalMeshes = model.meshes.size();
	const size_t totalImages = model.images.size();
	const size_t totalMaterials = model.materials.size();

	scene newScene{};
	newScene._meshes.resize(totalMeshes);
	newScene._images.resize(totalImages);
	newScene._materials.resize(totalMaterials);

	for (size_t i = 0; i < totalMeshes; ++i)
	{
		const size_t totalMeshPrimites = model.meshes[i].primitives.size();
		newScene._meshes[i]._primitives.resize(totalMeshPrimites);

		for (size_t k = 0; k < totalMeshPrimites; ++k)
		{
			auto& sceneMeshPrimitive = newScene._meshes[i]._primitives[k];
			const auto& primitive = model.meshes[i].primitives[k];

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