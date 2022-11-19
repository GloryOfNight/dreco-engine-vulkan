#pragma once
#include "core/containers/gltf/model.hxx"

#include <string_view>

class gltf_loader
{
public:
	static gltf::model loadModel(const std::string_view sceneFile);
};
