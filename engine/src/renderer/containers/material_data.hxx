#pragma once

#include <cstdint>

struct alignas(16) material_data
{
	uint32_t _baseColorIndex{UINT32_MAX};
	float _baseColorFactor[4]{1.F, 1.F, 1.F, 1.F};

	uint32_t _metallicRoughnessIndex{UINT32_MAX};
	float _metallicFactor{1.F};
	float _roughnessFactor{1.F};

	uint32_t _emissiveIndex{UINT32_MAX};
	float _emissiveFactor[3]{0.F, 0.F, 0.F};

	uint32_t _normalIndex{UINT32_MAX};
	float _normalScale{1.F};
};
static_assert(alignof(material_data) == 16, "must be aligned by 16");
