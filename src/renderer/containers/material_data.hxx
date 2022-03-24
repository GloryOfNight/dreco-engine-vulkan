#pragma once

#include <cstdint>

struct material_data // make sure all elemets alligned by 16
{
	uint32_t _baseColorIndex{UINT32_MAX};
	uint32_t _metallicRoughnessIndex{UINT32_MAX};
	uint32_t _normalIndex{UINT32_MAX};
	uint32_t _emissiveIndex{UINT32_MAX};
	// 16 ^
	float _baseColorFactor[4]{1.F, 1.F, 1.F, 1.F};
	// 16 ^
	float _emissiveFactor[3]{0.F, 0.F, 0.F};
	float _normalScale{1.F};
	// 16 ^
	float _roughnessFactor{1.F};
	float _metallicFactor{1.F};
	// 8 ^
};