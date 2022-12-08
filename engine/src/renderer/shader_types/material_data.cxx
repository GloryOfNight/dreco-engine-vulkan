#include "material_data.hxx"

material_data::material_data(const gltf::material& mat)
	: _baseColorFactor{vec4::narrow_construct(mat._pbrMetallicRoughness._baseColorFactor[0], mat._pbrMetallicRoughness._baseColorFactor[1], mat._pbrMetallicRoughness._baseColorFactor[2], mat._pbrMetallicRoughness._baseColorFactor[3])}
	, _emissiveFactor{vec3::narrow_construct(mat._emissive._factor[0], mat._emissive._factor[1], mat._emissive._factor[2])}
	, _hasBaseColor{mat._pbrMetallicRoughness._baseColorTexture._index != UINT32_MAX}
	, _hasEmissive{mat._emissive._index != UINT32_MAX}
	, _hasMetallicRoughness{mat._pbrMetallicRoughness._metallicRoughnessTexture._index != UINT32_MAX}
	, _hasNormal{mat._normal._index != UINT32_MAX}
	, _metallicFactor{static_cast<float>(mat._pbrMetallicRoughness._metallicFactor)}
	, _roughnessFactor{static_cast<float>(mat._pbrMetallicRoughness._roughnessFactor)}
	, _normalScale{static_cast<float>(mat._normal._scale)}
{
}
