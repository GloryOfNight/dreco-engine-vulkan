#include "mat3x4.hxx"
#include "vec3.hxx"

mat3x4::mat3x4() : _mat{0}
{
}

mat3x4::mat3x4(const float m[3][4])
	: _mat{{m[0][0], m[0][1], m[0][2], m[0][3]}, {m[1][0], m[1][1], m[1][2], m[1][3]},
		  {m[2][0], m[2][1], m[2][2], m[2][3]}}
{
}

constexpr float mat3x4::size() noexcept 
{
	return 12;
}

mat3x4 mat3x4::makeTranslation(const vec3& vec) 
{
	// clang-format off
	const float mat[3][4] = 
	{
		{ 1, 0, vec._x, 0 },
		{ 0, 1, vec._y, 0 },
		{ 0, 0, vec._z, 0 }
	};
	// clang-format on
	return mat3x4(mat);
}

mat3x4 mat3x4::makeRotation(const vec3& vec) 
{
	// TODO: write a rotation matrix
	return makeIdentity();
}

mat3x4 mat3x4::makeScale(const vec3& vec) 
{
	// clang-format off
	const float mat[3][4] = 
	{
		{ vec._x, 0, 0, 0 },
		{ 0, vec._y, 0, 0 },
		{ 0, 0, vec._z, 0 }
	};
	// clang-format on
	return mat3x4(mat);
}

mat3x4 mat3x4::makeIdentity()
{
	// clang-format off
	const float mat[3][4] = 
	{
		{ 1, 0, 0, 0 }, 
		{ 0, 1, 0, 0 }, 
		{ 0, 0, 1, 0 }
	};
	// clang-format on
	return mat3x4(mat);
}

mat3x4 operator*(const mat3x4& a, const mat3x4& b)
{
	// clang-format off
	const float m[3][4] =  
	{
		{ 
			a._mat[0][0] * b._mat[0][0] + a._mat[0][1] * b._mat[1][0] + a._mat[0][2] * b._mat[2][0], 
			a._mat[0][0] * b._mat[0][1] + a._mat[0][1] * b._mat[1][1] + a._mat[0][2] * b._mat[2][1],
			a._mat[0][0] * b._mat[0][2] + a._mat[0][1] * b._mat[1][2] + a._mat[0][2] * b._mat[2][2],
			a._mat[0][0] * b._mat[0][3] + a._mat[0][1] * b._mat[1][3] + a._mat[0][2] * b._mat[2][3] + a._mat[0][3]
		},
		{
			a._mat[1][0] * b._mat[0][0] + a._mat[1][1] * b._mat[1][0] + a._mat[1][2] * b._mat[2][0],
			a._mat[1][0] * b._mat[0][1] + a._mat[1][1] * b._mat[1][1] + a._mat[1][2] * b._mat[2][1],
			a._mat[1][0] * b._mat[0][2] + a._mat[1][1] * b._mat[1][2] + a._mat[1][2] * b._mat[2][2],
			a._mat[1][0] * b._mat[0][3] + a._mat[1][1] * b._mat[1][3] + a._mat[1][2] * b._mat[2][3] + a._mat[1][3]
		},
		{
			a._mat[2][0] * b._mat[0][0] + a._mat[2][1] * b._mat[1][0] + a._mat[2][2] * b._mat[2][0],
			a._mat[2][0] * b._mat[0][1] + a._mat[2][1] * b._mat[1][1] + a._mat[2][2] * b._mat[2][1],
			a._mat[2][0] * b._mat[0][2] + a._mat[2][1] * b._mat[1][2] + a._mat[2][2] * b._mat[2][2],
			a._mat[2][0] * b._mat[0][3] + a._mat[2][1] * b._mat[1][3] + a._mat[2][2] * b._mat[2][3] + a._mat[2][3]
		}
	};
	// clang-format on
	return mat3x4(m);
}