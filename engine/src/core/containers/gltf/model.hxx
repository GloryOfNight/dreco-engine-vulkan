#pragma once

#include "image.hxx"
#include "material.hxx"
#include "mesh.hxx"
#include "node.hxx"
#include "scene.hxx"

#include <vector>

namespace gltf
{
	struct model
	{
		std::string _rootPath;

		std::vector<mesh> _meshes;

		std::vector<material> _materials;

		std::vector<image> _images;

		uint32_t _sceneIndex{UINT32_MAX};
		std::vector<scene> _scenes;

		std::vector<node> _nodes;
	};
} // namespace gltf