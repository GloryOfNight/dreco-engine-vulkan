#pragma once
#include "math/vec2.hxx"
#include "math/vec3.hxx"
#include "math/vec4.hxx"

#include <cstdint>
#include <string>
#include <vector>

namespace de::gltf
{
	struct mesh
	{
		struct primitive
		{
			struct vertex
			{
				math::vec3 _pos;

				math::vec3 _normal;

				math::vec2 _texCoord;

				math::vec4 _color;
			};

			std::vector<vertex> _vertexes;
			std::vector<uint32_t> _indexes;

			uint32_t _material{UINT32_MAX};
		};
		std::string _name;
		std::vector<primitive> _primitives;
	};
} // namespace de::gltf