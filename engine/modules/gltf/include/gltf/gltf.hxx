#pragma once
#include "dreco.hxx"
#include "model.hxx"
#include "image.hxx"

#include <string_view>

namespace de::gltf
{
	DRECO_API de::gltf::model loadModel(const std::string_view sceneFile);

	DRECO_API de::gltf::image loadImage(const std::string_view imageFile);
} // namespace de::gltf