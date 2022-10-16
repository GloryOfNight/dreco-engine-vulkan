#include "material_data.hxx"


material_data::material_data(const gltf::material& mat)
{
	_baseColorIndex = mat._pbrMetallicRoughness._baseColorTexture._index != UINT32_MAX;
	_baseColorFactor[0] = mat._pbrMetallicRoughness._baseColorFactor[0];
	_baseColorFactor[1] = mat._pbrMetallicRoughness._baseColorFactor[1];
	_baseColorFactor[2] = mat._pbrMetallicRoughness._baseColorFactor[2];
	_baseColorFactor[3] = mat._pbrMetallicRoughness._baseColorFactor[3];

	_emissiveIndex = mat._emissive._index != UINT32_MAX;
	_emissiveFactor[0] = mat._emissive._factor[0];
	_emissiveFactor[1] = mat._emissive._factor[1];
	_emissiveFactor[2] = mat._emissive._factor[2];

	_metallicRoughnessIndex = mat._pbrMetallicRoughness._metallicRoughnessTexture._index != UINT32_MAX;
	_metallicFactor = mat._pbrMetallicRoughness._metallicFactor;
	_roughnessFactor = mat._pbrMetallicRoughness._roughnessFactor;

	_normalIndex = mat._normal._index != UINT32_MAX;
	_normalScale = mat._normal._scale;
}
