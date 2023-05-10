#pragma once

#include "euler.hxx"
#include "vec3.hxx"

namespace de::math
{
	struct transform
	{
		transform();
		transform(const vec3& translation, const euler& rotation, const vec3& scale);

		vec3 _translation;

		euler _rotation;

		vec3 _scale;

		transform operator+(const transform& other) const
		{
			return transform(_translation + other._translation, _rotation + other._rotation, _scale * other._scale);
		}

		transform& operator+=(const transform& other)
		{
			*this = *this + other;
			return *this;
		}
	};
} // namespace de::math