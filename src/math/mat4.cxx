#include "mat4.hxx"
#include "vec3.hxx"

mat4::mat4() : _mat{0}
{
}

mat4::mat4(const float m[4][4])
	: _mat{{m[0][0], m[0][1], m[0][2], m[0][3]}, {m[1][0], m[1][1], m[1][2], m[1][3]},
		  {m[2][0], m[2][1], m[2][2], m[2][3]}, {m[3][0], m[3][1], m[3][2], m[3][3]}}
{
}

constexpr float mat4::size() noexcept 
{
	return 16;
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
	// TODO: write a rotation matrix
	return makeIdentity();
}

mat4 mat4::makeScale(const vec3& vec) 
{
	// clang-format off
	const float mat[3][4] = 
	{
		{ vec._x, 0, 0, 0 },
		{ 0, vec._y, 0, 0 },
		{ 0, 0, vec._z, 0 }
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

mat4 operator*(const mat4& a, const mat4& b)
{
	// clang-format off
	const float m[4][4] =  
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
		},
		{
			0, 
			0, 
			0, 
			1
		}
	};
	// clang-format on
	return mat4(m);
}