#pragma once
#include "math/mat4.hxx"
#include "math/vec2.hxx"
#include "math/vec3.hxx"

#include <cstdint>
#include <vector>

namespace gltf
{
	struct material;
	struct mesh
	{
		struct primitive
		{
			struct vertex
			{
				vec3 _pos;

				vec3 _normal;

				vec2 _texCoord;
			};

			std::vector<vertex> _vertexes;
			std::vector<uint32_t> _indexes;

			uint32_t _material{UINT32_MAX};
		};
		std::vector<primitive> _primitives;
	};
} // namespace gltf
