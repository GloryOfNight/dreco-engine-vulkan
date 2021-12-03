#pragma once

#include "dreco.hxx"

struct DRECO_DECLSPEC vec3
{
	vec3();

	vec3(const float& x, const float& y, const float& z);

	static float dot(const vec3& first, const vec3& second);

	float _x;
	float _y;
	float _z;
};

vec3 operator+(const vec3& first, const vec3& second);
vec3 operator+=(vec3& first, const vec3& second);

vec3 operator*(const vec3& first, const vec3& second);

vec3 operator*(const vec3& first, const float value);