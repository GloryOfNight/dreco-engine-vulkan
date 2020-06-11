#pragma once
#include <stddef.h>

struct vec3;

struct mat3x4
{
	mat3x4();

	mat3x4(const float m[3][4]);

	static constexpr float size() noexcept;

	static mat3x4 makeTranslation(const vec3& vec);

	static mat3x4 makeRotation(const vec3& vec);

	static mat3x4 makeScale(const vec3& vec);
	
	static mat3x4 makeIdentity();

	float _mat[3][4];
};

mat3x4 operator*(const mat3x4& a, const mat3x4& b);