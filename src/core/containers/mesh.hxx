#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"

#include <cstdint>
#include <vector>

struct material;

struct mesh
{
	struct primitive
	{
		struct vertex
		{
			vec3 _pos;

			vec2 _texCoord;
		};

		std::vector<vertex> _vertexes;
		std::vector<uint32_t> _indexes;

		material* _material{nullptr};
	};
	std::vector<primitive> _primitives;
};