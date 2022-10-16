#pragma once

#include "core/containers/gltf/material.hxx"

#include <cstdint>

struct alignas(16) material_data
{
	material_data() = default;
	material_data(const gltf::material& m);

	bool _baseColorIndex{false};
	float _baseColorFactor[4]{1.F, 1.F, 1.F, 1.F};

	bool _metallicRoughnessIndex{false};
	float _metallicFactor{1.F};
	float _roughnessFactor{1.F};

	bool _emissiveIndex{false};
	float _emissiveFactor[3]{0.F, 0.F, 0.F};

	bool _normalIndex{false};
	float _normalScale{1.F};
};
static_assert(alignof(material_data) == 16, "must be aligned by 16");
