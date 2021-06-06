#pragma once
#include "renderer/containers/mesh_data.hxx"

class gltf_loader
{
public:
	static mesh_data loadScene(const char* sceneFile);
};
