#pragma once
#include "renderer/containers/mesh_data.hxx"
#include "renderer/containers/uniforms.hxx"

class vk_renderer;

class vk_mesh
{
public:
	vk_mesh(const vk_renderer* renderer);

private:
    mesh_data mesh;

    const vk_renderer* _renderer;
};