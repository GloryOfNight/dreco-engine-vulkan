#include "material_data.hxx"

material_data::material_data(const gltf::material& mat)
	: _baseColorFactor{mat._pbrMetallicRoughness._baseColorFactor}
	, _emissiveFactor{mat._emissive._factor}
	, _hasBaseColor{mat._pbrMetallicRoughness._baseColorTexture._index != UINT32_MAX}
	, _hasEmissive{mat._emissive._index != UINT32_MAX}
	, _hasMetallicRoughness{mat._pbrMetallicRoughness._metallicRoughnessTexture._index != UINT32_MAX}
	, _hasNormal{mat._normal._index != UINT32_MAX}
	, _metallicFactor{static_cast<float>(mat._pbrMetallicRoughness._metallicFactor)}
	, _roughnessFactor{static_cast<float>(mat._pbrMetallicRoughness._roughnessFactor)}
	, _normalScale{static_cast<float>(mat._normal._scale)}
{
}
