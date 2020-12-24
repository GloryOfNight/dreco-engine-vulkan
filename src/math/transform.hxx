#pragma once

#include "vec3.hxx"

struct transform
{
	transform() = default;
	transform(const vec3& translation, const vec3& rotation, const vec3& scale);

	vec3 _translation;

	vec3 _rotation;

	vec3 _scale;
};