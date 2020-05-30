#pragma once
#include <stddef.h>

struct mat3x4
{
	mat3x4();

	mat3x4(const float m[3][4]);

	static mat3x4 makeIdentityMatrix();

	/**
	1 # # #
	# 1 # #
	# # 1 #
	*/
	float _mat[3][4];
};

mat3x4 operator*(const mat3x4& a, const mat3x4& b);