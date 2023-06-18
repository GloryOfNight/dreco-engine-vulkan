#include "gltf.hxx"

#include "log/log.hxx"
#include "math/casts.hxx"
#include "math/mat4.hxx"

#include <algorithm>
#include <cstring>
#include <execution>
#include <filesystem>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tinygltf/tiny_gltf.h"

// t - for tiny, d - for dreco

static de::math::mat4 parseMatrix(const std::vector<double>& matrix)
{
	de::math::mat4 out = de::math::mat4::makeIdentity();

	if (matrix.size() == 16)
	{
		for (uint8_t i = 0; i < 4; ++i)
		{
			const uint8_t indx = i * 4;
			out[i][0] = matrix[indx];
			out[i][1] = matrix[indx + 1];
			out[i][2] = matrix[indx + 2];
			out[i][3] = matrix[indx + 3];
		}
	}
	return out;
}

static void parseScenes(const tinygltf::Model& tModel, de::gltf::model& dModel)
{
	dModel._sceneIndex = static_cast<uint32_t>(tModel.defaultScene);

	const size_t totalScenes = tModel.scenes.size();
	dModel._scenes.resize(totalScenes);
	for (size_t i = 0; i < totalScenes; ++i)
	{
		const auto& tScene = tModel.scenes[i];
		auto& dScene = dModel._scenes[i];

		dScene._name = std::move(tScene.name);

		const size_t sceneTotalNodes = tScene.nodes.size();
		dScene._nodes.resize(sceneTotalNodes);
		for (size_t k = 0; k < sceneTotalNodes; ++k)
		{
			dScene._nodes[k] = static_cast<uint32_t>(tScene.nodes[k]);
		}
	}
}

static void parseNodes(const tinygltf::Model& tModel, de::gltf::model& dModel)
{
	const size_t totalNodes = tModel.nodes.size();
	dModel._nodes.resize(totalNodes);
	for (size_t i = 0; i < totalNodes; ++i)
	{
		const auto& tNode = tModel.nodes[i];
		auto& dNode = dModel._nodes[i];

		dNode._name = std::move(tNode.name);

		const size_t totalChildren = tNode.children.size();
		dNode._children.resize(totalChildren);
		for (size_t i = 0; i < totalChildren; ++i)
		{
			dNode._children[i] = static_cast<uint32_t>(tNode.children[i]);
		}

		dNode._mesh = static_cast<uint32_t>(tNode.mesh);

		dNode._matrix = de::math::mat4::makeIdentity();
		if (tNode.matrix.empty())
		{
			if (tNode.translation.size() == 3)
			{
				const auto translation = de::math::vec3(tNode.translation[0], tNode.translation[1], tNode.translation[2]);
				dNode._transform._translation = translation;
			}
			if (tNode.rotation.size() == 4)
			{
				const auto quat = de::math::quaternion(tNode.rotation[0], tNode.rotation[1], tNode.rotation[2], tNode.rotation[3]);
				dNode._transform._rotation = de::math::euler_cast(quat);
			}
			if (tNode.scale.size() == 3)
			{
				const auto scale = de::math::vec3::narrow_construct(tNode.scale[0], tNode.scale[1], tNode.scale[2]);
				dNode._transform._scale = scale;
			}
			dNode._matrix = de::math::mat4::makeTransform(dNode._transform);
		}
		else
		{
			dNode._matrix = parseMatrix(tNode.matrix);
			dNode._transform = de::math::transform_cast<de::math::transform>(dNode._matrix);
		}
	}
}

static void parseMeshes(const tinygltf::Model& tModel, de::gltf::model& dModel)
{
	const size_t totalMeshes = tModel.meshes.size();
	dModel._meshes.resize(totalMeshes);
	for (size_t i = 0; i < totalMeshes; ++i)
	{
		const auto& tMesh = tModel.meshes[i];
		auto& dMesh = dModel._meshes[i];

		dMesh._name = std::move(tMesh.name);

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
			uint32_t colorAccessor{UINT32_MAX};
			const uint32_t indexAccessor{static_cast<uint32_t>(tPrimitive.indices)};

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
				else if (attr.first == "COLOR_0")
				{
					colorAccessor = attr.second;
				}
			}

			dPrimitive._vertexes.resize(tModel.accessors[vertPosAccessor].count);

			if (indexAccessor != UINT32_MAX)
				dPrimitive._indexes.resize(tModel.accessors[indexAccessor].count);

			const std::array<uint32_t, 5> usedAccessors{vertPosAccessor, indexAccessor, texCoordAccessor, normalAccessor, colorAccessor};
			for (const uint32_t accessorIndex : usedAccessors)
			{
				if (accessorIndex == UINT32_MAX)
					continue;
				const auto& accessor{tModel.accessors[accessorIndex]};
				const auto& bufferView{tModel.bufferViews[accessor.bufferView]};
				const auto& buffer{tModel.buffers[bufferView.buffer]};
				const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t q = 0; q < accessor.count; ++q)
				{
					if (accessorIndex == vertPosAccessor && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						de::math::vec3& pos{dPrimitive._vertexes[q]._pos};
						pos._x = positions[q * 3 + 0];
						pos._y = positions[q * 3 + 1];
						pos._z = positions[q * 3 + 2];
					}
					else if (accessorIndex == indexAccessor)
					{
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						{
							const uint16_t* indexPos = reinterpret_cast<const uint16_t*>(positions);
							dPrimitive._indexes[q] = indexPos[q];
						}
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
						{
							const uint32_t* indexPos = reinterpret_cast<const uint32_t*>(positions);
							dPrimitive._indexes[q] = indexPos[q];
						}
					}
					else if (accessorIndex == texCoordAccessor)
					{
						de::math::vec2& texCoor{dPrimitive._vertexes[q]._texCoord};
						texCoor._u = positions[q * 2 + 0];
						texCoor._v = positions[q * 2 + 1];
					}
					else if (accessorIndex == normalAccessor)
					{
						de::math::vec3& normal{dPrimitive._vertexes[q]._normal};
						normal._x = positions[q * 3 + 0];
						normal._y = positions[q * 3 + 1];
						normal._z = positions[q * 3 + 2];
					}
					else if (accessorIndex == colorAccessor)
					{
						de::math::vec4& color{dPrimitive._vertexes[q]._color};
						const uint8_t size = accessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						{
							color._r = positions[q * size + 0];
							color._g = positions[q * size + 1];
							color._b = positions[q * size + 2];
							if (size == 4)
							{
								color._a = positions[q * size + 3];
							}
						}
					}
				}
			}
		}
	}
}

static void parseMaterials(const tinygltf::Model& tModel, de::gltf::model& dModel)
{
	const size_t totalMaterials = tModel.materials.size();
	dModel._materials.resize(totalMaterials);
	for (size_t i = 0; i < totalMaterials; ++i)
	{
		const auto& tMat = tModel.materials[i];
		auto& dMat = dModel._materials[i];

		dMat._doubleSided = tMat.doubleSided;
		dMat._normal._index = static_cast<uint32_t>(tMat.normalTexture.index);
		dMat._normal._scale = tMat.normalTexture.scale;

		dMat._emissive._index = static_cast<uint32_t>(tMat.emissiveTexture.index);
		dMat._emissive._factor = std::array<double, 3>{tMat.emissiveFactor[0], tMat.emissiveFactor[1], tMat.emissiveFactor[2]};

		dMat._occlusion._index = static_cast<uint32_t>(tMat.occlusionTexture.index);
		dMat._occlusion._strength = tMat.occlusionTexture.strength;

		std::memcpy(dMat._pbrMetallicRoughness._baseColorFactor.data(), tMat.pbrMetallicRoughness.baseColorFactor.data(), sizeof(double) * 4);
		dMat._pbrMetallicRoughness._baseColorTexture._index = static_cast<uint32_t>(tMat.pbrMetallicRoughness.baseColorTexture.index);
		dMat._pbrMetallicRoughness._metallicFactor = tMat.pbrMetallicRoughness.metallicFactor;
		dMat._pbrMetallicRoughness._metallicRoughnessTexture._index = static_cast<uint32_t>(tMat.pbrMetallicRoughness.metallicRoughnessTexture.index);
		dMat._pbrMetallicRoughness._roughnessFactor = tMat.pbrMetallicRoughness.roughnessFactor;
	}
}

static void parseImages(const tinygltf::Model& tModel, de::gltf::model& dModel)
{
	const size_t totalImages = tModel.images.size();

	dModel._images.resize(totalImages);
	for (size_t i = 0; i < totalImages; ++i)
	{
		dModel._images[i]._uri = tModel.images[i].uri;
	}

	const auto asyncImageLoad = [&dModel](de::gltf::image& image)
	{
		image = std::move(de::gltf::loadImage(dModel._rootPath + '/' + image._uri));
	};
	std::for_each(std::execution::par, dModel._images.begin(), dModel._images.end(), asyncImageLoad);
}

de::gltf::model de::gltf::loadModel(const std::string_view sceneFile)
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
	parseMaterials(tModel, dModel);
	parseMeshes(tModel, dModel);
	parseImages(tModel, dModel);

	return dModel;
}

DRECO_API de::gltf::image de::gltf::loadImage(const std::string_view imageFile)
{
	constexpr auto components = 4U;

	int width, heigth, channels;
	const auto stbiPixels = stbi_load(imageFile.data(), &width, &heigth, &channels, components);

	gltf::image image;
	if (stbiPixels)
	{
		const size_t pixelCount = width * heigth * components;

		image._width = width;
		image._height = heigth;
		image._channels = channels;
		image._components = components;

		image._pixels.resize(pixelCount);
		std::memmove(image._pixels.data(), stbiPixels, pixelCount);

		delete[] stbiPixels;
	}
	else
	{
		DE_LOG(Error, "Failed to load image: %s", image._uri.data());
		image = de::gltf::image::makePlaceholder(256, 256);
	}
	return std::move(image);
}