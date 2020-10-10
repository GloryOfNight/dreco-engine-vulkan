#pragma once
#include "vertex.hxx"

#include <stdint.h>
#include <vector>

struct mesh_data
{
	std::vector<vertex> _vertexes;
	std::vector<uint32_t> _indexes;

	static mesh_data createSprite()
	{
		// clang-format off
		const mesh_data mesh
		{
			{
				{vec3(-0.5, 0.5, 0.0)}, 
				{vec3(-0.5, -0.5, 0.0)}, 
				{vec3(0.5, -0.5, 0.0)}, 
				{vec3(0.5, 0.5, 0.0)}
			},
			{
				0, 1, 2,
				0, 2, 3    
			}
		};
		// clang-format on
		return mesh;
	}
};