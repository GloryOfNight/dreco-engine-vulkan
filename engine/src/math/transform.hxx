#pragma once

#include "rotator.hxx"
#include "vectors.hxx"

namespace de::math
{
	struct DRECO_API transform
	{
		transform();
		transform(const vec3& translation, const rotator& rotation, const vec3& scale);

		vec3 _translation;

		rotator _rotation;

		vec3 _scale;
	};
} // namespace de::math