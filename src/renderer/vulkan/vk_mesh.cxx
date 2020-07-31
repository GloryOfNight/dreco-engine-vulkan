#include "vk_mesh.hxx"
#include "vk_renderer.hxx"

vk_mesh::vk_mesh(const vk_renderer* renderer)
 : mesh(mesh_data::createSprite()), _renderer(renderer)
{
}