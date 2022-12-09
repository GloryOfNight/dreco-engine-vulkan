// forward declarations for vectors
#pragma once
#include "dreco.hxx"
#include "vec2.hxx"
#include "vec3.hxx"
#include "vec4.hxx"

namespace de::math
{
	template <typename T>
	struct DRECO_API vec2t;
	using vec2i = vec2t<int32_t>;
	using vec2d = vec2t<double>;
	using vec2f = vec2t<float>;
	using vec2 = vec2f;

	template <typename T>
	struct DRECO_API vec3t;
	using vec3i = vec3t<int32_t>;
	using vec3d = vec3t<double>;
	using vec3f = vec3t<float>;
	using vec3 = vec3f;

	template <typename T>
	struct DRECO_API vec4t;
	using vec4i = vec4t<int32_t>;
	using vec4d = vec4t<double>;
	using vec4f = vec4t<float>;
	using vec4 = vec4f;
} // namespace de::math