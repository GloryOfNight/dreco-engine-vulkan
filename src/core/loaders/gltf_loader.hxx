#pragma once
#include "renderer/containers/mesh_data.hxx"

#include <string_view>

class gltf_loader
{
public:
	static std::vector<mesh_data> loadScene(const std::string_view& sceneFile);
};
