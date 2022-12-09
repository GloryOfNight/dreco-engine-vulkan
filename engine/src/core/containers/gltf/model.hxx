#pragma once

#include "image.hxx"
#include "material.hxx"
#include "mesh.hxx"
#include "node.hxx"
#include "scene.hxx"

#include <vector>

namespace de::gltf
{
	struct model
	{
		std::string _rootPath;

		std::vector<de::gltf::mesh> _meshes;

		std::vector<de::gltf::material> _materials;

		std::vector<de::gltf::image> _images;

		uint32_t _sceneIndex{UINT32_MAX};
		std::vector<de::gltf::scene> _scenes;

		std::vector<de::gltf::node> _nodes;
	};
} // namespace de::gltf