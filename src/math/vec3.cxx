#include "vec3.hxx"

vec3::vec3()
	: _x{0}
	, _y{0}
	, _z{0}
{
}

vec3::vec3(const float& x, const float& y, const float& z)
	: _x{x}
	, _y{y}
	, _z{z}
{
}

float vec3::dot(const vec3& first, const vec3& second)
{
	return (first._x * second._x) + (first._y * second._y) + (first._z * second._z);
}

vec3 operator+(const vec3& first, const vec3& second)
{
	return vec3(first._x + second._x, first._y + second._y, first._z + second._z);
}

vec3 operator*(const vec3& first, const vec3& second)
{
	return vec3(first._x * second._x, first._y * second._y, first._z * second._z);
}

vec3 operator*(const vec3& first, const float value)
{
	return vec3(first._x * value, first._y * value, first._z * value);
}