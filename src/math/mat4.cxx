#include "mat4.hxx"

#include "rotator.hxx"
#include "transform.hxx"
#include "vec3.hxx"

#include <cmath>
#include <cstring>

mat4::mat4()
	: _mat{}
{
}

mat4::mat4(const mat4d& mat)
	: _mat{mat}
{
}

constexpr float mat4::size() noexcept
{
	return 16;
}

mat4 mat4::makeTransform(const transform& t)
{
	return makeScale(t._scale) * makeTranslation(t._translation) * makeRotation(t._rotation);
}

mat4 mat4::makeTranslation(const vec3& vec)
{
	// clang-format off
	const mat4d mat =
		{{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{vec._x, vec._y, vec._z, 1}
		}};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeRotation(const rotator& rot)
{
	const rotator radians = rot.toRadians();

	float cos_x = std::cos(radians._pitch);
	float sin_x = std::sin(radians._pitch);

	float cos_y = std::cos(radians._yaw);
	float sin_y = std::sin(radians._yaw);

	float cos_z = std::cos(radians._roll);
	float sin_z = std::sin(radians._roll);

	// clang-format off
	const mat4d matXraw =
		{{
			{1, 0, 0, 0},
			{0, cos_x, -sin_x, 0},
			{0, sin_x, cos_x, 0},
			{0, 0, 0, 1}
		}};
	mat4 matX(matXraw);

	const mat4d matYraw =
		{{
			{cos_y, 0, sin_y, 0},
			{0, 1, 0, 0},
			{-sin_y, 0, cos_y, 0},
			{0, 0, 0, 1}
		}};
	mat4 matY(matYraw);

	const mat4d matZraw =
		{{
			{cos_z, -sin_z, 0, 0},
			{sin_z, cos_z, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		}};
	mat4 matZ(matZraw);
	// clang-format on

	return matZ * matY * matX;
}

mat4 mat4::makeScale(const vec3& vec)
{
	// clang-format off
	const mat4d mat =
		{{
			{vec._x, 0, 0, 0},
			{0, vec._y, 0, 0},
			{0, 0, vec._z, 0},
			{0, 0, 0, 1}
		}};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeIdentity()
{
	// clang-format off
	const mat4d mat = 
		{{
			{ 1, 0, 0, 0 }, 
			{ 0, 1, 0, 0 }, 
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		}};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeProjection(const float near, const float far, const float aspect, const float fov)
{
	float tanHalfFov = std::tan(fov / 2.F);

	mat4d mat{};
	mat[0][0] = 1 / (aspect * tanHalfFov);
	mat[1][1] = (1 / (aspect * tanHalfFov)) * -aspect;
	mat[2][2] = (far + near) / (far - near);
	mat[2][3] = 1;
	mat[3][2] = -(far * near) / (far - near);

	return mat4(mat);
}

mat4 operator*(const mat4& a, const mat4& b)
{
	// clang-format off
	const mat4::mat4d mat =  
	{{
		{ 
			a._mat[0][0] * b._mat[0][0] + a._mat[0][1] * b._mat[1][0] + a._mat[0][2] * b._mat[2][0] + a._mat[0][3] * b._mat[3][0], 
			a._mat[0][0] * b._mat[0][1] + a._mat[0][1] * b._mat[1][1] + a._mat[0][2] * b._mat[2][1] + a._mat[0][3] * b._mat[3][1],
			a._mat[0][0] * b._mat[0][2] + a._mat[0][1] * b._mat[1][2] + a._mat[0][2] * b._mat[2][2] + a._mat[0][3] * b._mat[3][2],
			a._mat[0][0] * b._mat[0][3] + a._mat[0][1] * b._mat[1][3] + a._mat[0][2] * b._mat[2][3] + a._mat[0][3] * b._mat[3][3]
		},
		{
			a._mat[1][0] * b._mat[0][0] + a._mat[1][1] * b._mat[1][0] + a._mat[1][2] * b._mat[2][0] + a._mat[1][3] * b._mat[3][0],
			a._mat[1][0] * b._mat[0][1] + a._mat[1][1] * b._mat[1][1] + a._mat[1][2] * b._mat[2][1] + a._mat[1][3] * b._mat[3][1],
			a._mat[1][0] * b._mat[0][2] + a._mat[1][1] * b._mat[1][2] + a._mat[1][2] * b._mat[2][2] + a._mat[1][3] * b._mat[3][2],
			a._mat[1][0] * b._mat[0][3] + a._mat[1][1] * b._mat[1][3] + a._mat[1][2] * b._mat[2][3] + a._mat[1][3] * b._mat[3][3]
		},
		{
			a._mat[2][0] * b._mat[0][0] + a._mat[2][1] * b._mat[1][0] + a._mat[2][2] * b._mat[2][0] + a._mat[2][3] * b._mat[3][0],
			a._mat[2][0] * b._mat[0][1] + a._mat[2][1] * b._mat[1][1] + a._mat[2][2] * b._mat[2][1] + a._mat[2][3] * b._mat[3][1],
			a._mat[2][0] * b._mat[0][2] + a._mat[2][1] * b._mat[1][2] + a._mat[2][2] * b._mat[2][2] + a._mat[2][3] * b._mat[3][2],
			a._mat[2][0] * b._mat[0][3] + a._mat[2][1] * b._mat[1][3] + a._mat[2][2] * b._mat[2][3] + a._mat[2][3] * b._mat[3][3]
		},
		{
			a._mat[3][0] * b._mat[0][0] + a._mat[3][1] * b._mat[1][0] + a._mat[3][2] * b._mat[2][0] + a._mat[3][3] * b._mat[3][0],
			a._mat[3][0] * b._mat[0][1] + a._mat[3][1] * b._mat[1][1] + a._mat[3][2] * b._mat[2][1] + a._mat[3][3] * b._mat[3][1],
			a._mat[3][0] * b._mat[0][2] + a._mat[3][1] * b._mat[1][2] + a._mat[3][2] * b._mat[2][2] + a._mat[3][3] * b._mat[3][2],
			a._mat[3][0] * b._mat[0][3] + a._mat[3][1] * b._mat[1][3] + a._mat[3][2] * b._mat[2][3] + a._mat[3][3] * b._mat[3][3]
		}
	}};
	// clang-format on
	return mat4(mat);
}

vec3 mat4::getAxis(const matAxis axis) const
{
	switch (axis)
	{
	case matAxis::X:
		return vec3(_mat[0][0], _mat[0][1], _mat[0][2]);
	case matAxis::Y:
		return vec3(_mat[1][0], _mat[1][1], _mat[1][2]);
	case matAxis::Z:
		return vec3(_mat[2][0], _mat[2][1], _mat[2][2]);
	}
	return vec3();
}