#pragma once
#include "image.hxx"
#include "material.hxx"
#include "mesh.hxx"

#include <string>
#include <vector>

struct scene
{
	std::string _sceneRootPath;

	std::vector<mesh> _meshes;

	std::vector<material> _materials;

	std::vector<image> _images;
};