#pragma once
#include "vertex.hxx"

#include <cstdint>
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
				{vec3(-0.5, -0.5, 0.0), vec2{1.0F, 0.0F}}, 
				{vec3(0.5, -0.5, 0.0), vec2{0.0F, 0.0F}}, 
				{vec3(0.5, 0.5, 0.0), vec2{0.0F, 1.0F}}, 
				{vec3(-0.5, 0.5, 0.0), vec2{1.0F, 1.0F}}
			},
			{
				0, 1, 2,
				0, 2, 3
			}
		};
		// clang-format on
		return mesh;
	}

	static mesh_data createBox()
	{
		// clang-format off
		const mesh_data mesh
		{
			{
				{vec3(-0.5, 0.5, 0.5), vec2{1.0F, 1.0F}}, 
				{vec3(-0.5, -0.5, 0.5), vec2{1.0F, 0.0F}}, 
				{vec3(0.5, -0.5, 0.5), vec2{0.0F, 0.0F}}, 
				{vec3(0.5, 0.5, 0.5), vec2{0.0F, 1.0F}},
				{vec3(-0.5, 0.5, -0.5), vec2{1.0F, 1.0F}}, 
				{vec3(-0.5, -0.5, -0.5), vec2{1.0F, 0.0F}}, 
				{vec3(0.5, -0.5, -0.5),vec2{0.0F, 0.0F}}, 
				{vec3(0.5, 0.5, -0.5), vec2{0.0F, 1.0F}}
			},
			{
				0, 1, 3, 3, 1, 2,
				1, 5, 2, 2, 5, 6,
				5, 4, 6, 6, 4, 7,
				4, 0, 7, 7, 0, 3,
				3, 2, 7, 7, 2, 6,
				4, 5, 0, 0, 5, 1
			}
		};
		// clang-format on
		return mesh;
	}
};