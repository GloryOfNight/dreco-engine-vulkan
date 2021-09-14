#pragma once
#include "core/containers/scene.hxx"

#include <string_view>

class gltf_loader
{
public:
	static scene loadScene(const std::string_view& sceneFile);
};
