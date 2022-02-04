#pragma once
#include <cstdint>
#include <array>

namespace gltf
{
	struct base_texture
	{
		uint32_t _index{UINT32_MAX};
	};

	struct normal_texture : public base_texture
	{
		double _scale{1.0};
	};

	struct occlusion_texture : public base_texture
	{
		double _strength{1.0};
	};

	struct pbr_metallic_roughness
	{
		std::array<double, 4> _baseColorFactor = {1.0, 1.0, 1.0, 1.0};

		base_texture _baseColorTexture;

		double _metallicFactor{1.0};

		base_texture _metallicRoughnessTexture;

		double _roughnessFactor{1.0};
	};

	struct material
	{
		bool _doubleSided{true};

		normal_texture _normalTexture;

		base_texture _emissiveTexture;

		occlusion_texture _occlusionTexture;

		pbr_metallic_roughness _pbrMetallicRoughness;
	};
} // namespace gltf