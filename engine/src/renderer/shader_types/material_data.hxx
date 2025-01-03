#pragma once

#include "gltf/material.hxx"
#include "math/vec3.hxx"
#include "math/vec4.hxx"

struct material_data
{
	material_data() = default;
	material_data(const de::gltf::material& m);

	de::math::vec4 _baseColorFactor;
	de::math::vec3 _emissiveFactor;

	alignas(4) bool _hasBaseColor{false};
	alignas(4) bool _hasEmissive{false};
	alignas(4) bool _hasMetallicRoughness{false};
	alignas(4) bool _hasNormal{false};

	float _metallicFactor{1.F};
	float _roughnessFactor{1.F};
	float _normalScale{1.F};
};
