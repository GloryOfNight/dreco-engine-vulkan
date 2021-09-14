#pragma once
#include "mesh.hxx"
#include "material.hxx"
#include "image.hxx"

#include <vector>

struct scene
{
    std::vector<mesh> _meshes;

    std::vector<material> _materials;

    std::vector<image> _images;
};