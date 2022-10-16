#include "material_data.hxx"


material_data::material_data(const gltf::material& mat)
{
	_hasBaseColor = mat._pbrMetallicRoughness._baseColorTexture._index != UINT32_MAX;
	_baseColorFactor._x = mat._pbrMetallicRoughness._baseColorFactor[0];
	_baseColorFactor._y = mat._pbrMetallicRoughness._baseColorFactor[1];
	_baseColorFactor._z = mat._pbrMetallicRoughness._baseColorFactor[2];
	_baseColorFactor._w = mat._pbrMetallicRoughness._baseColorFactor[3];

	_hasEmissive = mat._emissive._index != UINT32_MAX;
	_emissiveFactor._x = mat._emissive._factor[0];
	_emissiveFactor._y = mat._emissive._factor[1];
	_emissiveFactor._z = mat._emissive._factor[2];

	_hasMetallicRoughness = mat._pbrMetallicRoughness._metallicRoughnessTexture._index != UINT32_MAX;
	_metallicFactor = mat._pbrMetallicRoughness._metallicFactor;
	_roughnessFactor = mat._pbrMetallicRoughness._roughnessFactor;

	_hasNormal = mat._normal._index != UINT32_MAX;
	_normalScale = mat._normal._scale;
}
