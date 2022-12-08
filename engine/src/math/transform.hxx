#pragma once

#include "vec3.hxx"
#include "rotator.hxx"

struct DRECO_API transform
{
	transform();
	transform(const vec3& translation, const rotator& rotation, const vec3& scale);

	vec3 _translation;

	rotator _rotation;

	vec3 _scale;
};