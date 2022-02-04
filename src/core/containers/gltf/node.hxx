#pragma once

#include "math/mat4.hxx"
#include "math/quaternion.hxx"
#include "math/vec3.hxx"

#include <cstdint>

namespace gltf
{
	struct node
	{
		std::vector<uint32_t> _children;

		uint32_t _mesh{UINT32_MAX};

		mat4 _matrix;
	};
} // namespace gltf