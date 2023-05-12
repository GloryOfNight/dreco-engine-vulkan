#pragma once

#include "math/transform.hxx"
#include "math/mat4.hxx"

#include <cstdint>
#include <string>
#include <vector>

namespace de::gltf
{
	struct node
	{
		std::string _name;

		std::vector<uint32_t> _children;

		uint32_t _mesh{UINT32_MAX};

		math::transform _transform;
		math::mat4 _matrix;
	};
} // namespace de::gltf