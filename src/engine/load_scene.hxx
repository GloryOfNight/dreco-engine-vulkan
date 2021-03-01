#pragma once

#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/vertex.hxx"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#include <vector>

static mesh_data loadScene(const char* sceneFile)
{
	using namespace tinygltf;

	Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	const bool result = loader.LoadASCIIFromFile(&model, &err, &warn, sceneFile);
	if (!result)
	{
		return {};
	}

	std::vector<mesh_data> meshes;

	for (const auto& mesh : model.meshes)
	{
		for (const auto& primitive : mesh.primitives)
		{
			uint32_t vertPosAccessor{UINT32_MAX};
			uint32_t texCoordAccessor{UINT32_MAX};
			uint32_t indexAccessor{static_cast<uint32_t>(primitive.indices)};
			for (const auto& attr : primitive.attributes)
			{
				if (attr.first == "POSITION")
				{
					vertPosAccessor = attr.second;
				}
				else if (attr.first == "TEXCOORD_0")
				{
					texCoordAccessor = attr.second;
				}
			}

			mesh_data newMesh{};
			newMesh._vertexes.resize(model.accessors[vertPosAccessor].count);
			newMesh._indexes.resize(model.accessors[indexAccessor].count);

			const size_t accessorsSize{model.accessors.size()};
			for (uint32_t i = 0; i < accessorsSize; ++i)
			{
				TINYGLTF_TYPE_SCALAR;
				const auto& accessor{model.accessors[i]};
				const auto& bufferView{model.bufferViews[accessor.bufferView]};
				const auto& buffer{model.buffers[bufferView.buffer]};

				const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t k = 0; k < accessor.count; ++k)
				{
					if (i == vertPosAccessor)
					{
						vec3& pos{newMesh._vertexes[k]._pos};
						pos._x = positions[k * 3 + 0];
						pos._y = positions[k * 3 + 1];
						pos._z = positions[k * 3 + 2];
					}
					else if (i == texCoordAccessor)
					{
						vec2& texCoor{newMesh._vertexes[k]._texCoord};
						texCoor._x = positions[k * 2 + 0];
						texCoor._y = positions[k * 2 + 1];
					}
					else if (i == indexAccessor)
					{
						const uint32_t* indexPos = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						newMesh._indexes[k] = indexPos[k];
					}
				}
			}

			return newMesh;
		}
	}
	return mesh_data{};
}