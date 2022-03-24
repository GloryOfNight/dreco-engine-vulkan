#pragma once

#include "vec3.hxx"
#include "rotator.hxx"

struct transform
{
	transform();
	transform(const vec3& translation, const rotatorDeg& rotation, const vec3& scale);

	vec3 _translation;

	rotatorDeg _rotation;

	vec3 _scale;
};