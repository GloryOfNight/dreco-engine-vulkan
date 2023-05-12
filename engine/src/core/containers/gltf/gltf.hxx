#pragma once
#include "dreco.hxx"
#include "model.hxx"

#include <string_view>

namespace de::gltf
{
	DRECO_API de::gltf::model loadModel(const std::string_view sceneFile);
} // namespace de::gltf