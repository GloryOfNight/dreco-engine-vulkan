#pragma once

#include "dreco.h"

struct DRECO_DECLSPEC vec3
{
	vec3();

	vec3(const float& x, const float& y, const float& z);

	float _x;
	float _y;
	float _z;
};

vec3 operator+(const vec3& first, const vec3& second);