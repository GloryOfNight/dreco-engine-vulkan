#pragma once
#include <stddef.h>

struct vec3;
struct transform;

struct mat4
{
	mat4();

	mat4(const float m[4][4]);

	static constexpr float size() noexcept;

	static mat4 makeTransform(const transform& t);

	static mat4 makeTranslation(const vec3& vec);

	static mat4 makeRotation(const vec3& vec);

	static mat4 makeScale(const vec3& vec);

	static mat4 makeIdentity();

	static mat4 makeProjection(const float near, const float far, const float aspect, const float fov);

	float _mat[4][4];
};

mat4 operator*(const mat4& a, const mat4& b);