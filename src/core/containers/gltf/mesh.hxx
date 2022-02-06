#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"
#include "math/vec4.hxx"

#include <cstdint>
#include <vector>

namespace gltf
{
	struct mesh
	{
		struct primitive
		{
			struct vertex
			{
				vec3 _pos;

				vec3 _normal;

				vec2 _texCoord;

				vec4 _color;
			};

			std::vector<vertex> _vertexes;
			std::vector<uint32_t> _indexes;

			uint32_t _material{UINT32_MAX};
		};
		std::vector<primitive> _primitives;
	};
} // namespace gltf
