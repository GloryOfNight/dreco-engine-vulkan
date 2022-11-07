#pragma once

#include "core/containers/gltf/material.hxx"
#include "math/vec.hxx"

struct material_data
{
	material_data() = default;
	material_data(const gltf::material& m);

	vec4 _baseColorFactor;
	vec3 _emissiveFactor;

	alignas(4) bool _hasBaseColor{false};
	alignas(4) bool _hasEmissive{false};
	alignas(4) bool _hasMetallicRoughness{false};
	alignas(4) bool _hasNormal{false};

	alignas(4) float _metallicFactor{1.F};
	alignas(4) float _roughnessFactor{1.F};
	alignas(4) float _normalScale{1.F};
};
