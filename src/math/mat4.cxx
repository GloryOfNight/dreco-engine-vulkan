#include "mat4.hxx"

#include "transform.hxx"
#include "vec3.hxx"

#include <cmath>
#include <cstring>

mat4::mat4()
{
}

mat4::mat4(const float m[4][4])
{
	std::memcpy(_mat, m, sizeof(mat4));
}

constexpr float mat4::size() noexcept
{
	return 16;
}

mat4 mat4::makeTransform(const transform& t)
{
	return makeScale(t._scale) * makeRotation(t._rotation) * makeTranslation(t._translation);
}

mat4 mat4::makeTranslation(const vec3& vec)
{
	// clang-format off
	const float mat[4][4] = 
	{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0},
		{ vec._x, vec._y, vec._z, 1 },
	};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeRotation(const vec3& vec)
{
	float cos_x = std::cos(vec._x);
	float sin_x = std::sin(vec._x);

	// clang-format off
	const float matXraw[4][4] =
	{
		{1, 0, 0, 0},
		{0, cos_x, -sin_x, 0},
		{0, sin_x, cos_x, 0},
		{0, 0, 0, 1},
	};

	float cos_y = std::cos(vec._y);
	float sin_y = std::sin(vec._y);

	const float matYraw[4][4] =
	{
		{cos_y, 0, sin_y, 0},
		{0, 1, 0, 0},
		{-sin_y, 0, cos_y, 0},
		{0, 0, 0, 1}
	};

	float cos_z = std::cos(vec._z);
	float sin_z = std::sin(vec._z);

	const float matZraw[4][4] =
	{
		{cos_z, -sin_z, 0, 0},
		{sin_z, cos_z, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};
	// clang-format on

	mat4 matX(matXraw);
	mat4 matY(matYraw);
	mat4 matZ(matZraw);

	return matX * matY * matZ;
}

mat4 mat4::makeScale(const vec3& vec)
{
	// clang-format off
	const float mat[4][4] = 
	{
		{ vec._x, 0, 0, 0 },
		{ 0, vec._y, 0, 0 },
		{ 0, 0, vec._z, 0 }, 
		{ 0, 0, 0, 1 }
	};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeIdentity()
{
	// clang-format off
	const float mat[4][4] = 
	{
		{ 1, 0, 0, 0 }, 
		{ 0, 1, 0, 0 }, 
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	};
	// clang-format on
	return mat4(mat);
}

mat4 mat4::makeProjection(const float near, const float far, const float aspect, const float fov)
{
	// not a actual projection, for now
	// clang-format off
	const float mat[4][4] = 
	{
		{ 1, 0, 0, 0 }, 
		{ 0, -1, 0, 0 }, 
		{ 0, 0, 1.0f / 2.0f, 1.0f / 2.0f },
		{ 0, 0, 0, 1 }
	};
	// clang-format on
	return mat;
}

mat4 operator*(const mat4& a, const mat4& b)
{
	// clang-format off
	const float m[4][4] =  
	{
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
	};
	// clang-format on
	return mat4(m);
}