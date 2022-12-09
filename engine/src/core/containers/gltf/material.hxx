#pragma once
#include <array>
#include <cstdint>

namespace de::gltf
{
	struct base_texture
	{
		uint32_t _index{UINT32_MAX};
	};

	struct normal : public base_texture
	{
		double _scale{1.0};
	};

	struct occlusion : public base_texture
	{
		double _strength{1.0};
	};

	struct emissive : public base_texture
	{
		std::array<double, 3> _factor = {1.0, 1.0, 1.0};
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

		normal _normal;

		emissive _emissive;

		occlusion _occlusion;

		pbr_metallic_roughness _pbrMetallicRoughness;
	};
} // namespace de::gltf