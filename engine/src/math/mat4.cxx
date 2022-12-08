#include "mat4.hxx"

#include "quaternion.hxx"
#include "rotator.hxx"
#include "transform.hxx"
#include "vec3.hxx"

#include <cmath>
#include <cstring>

mat4::mat4(const mat4d& mat)
	: _mat{mat}
{
}

mat4::mat4(mat4d&& mat)
	: _mat{std::move(mat)}
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
	const auto rotRad = rot.toRadians();

	float cos_x = std::cos(rotRad._x);
	float sin_x = std::sin(rotRad._x);

	float cos_y = std::cos(rotRad._y);
	float sin_y = std::sin(rotRad._y);

	float cos_z = std::cos(rotRad._z);
	float sin_z = std::sin(rotRad._z);

	mat4 matX;
	mat4 matY;
	mat4 matZ;

	// clang-format off
	{
		{
			mat4d matXraw =
				{{
					{1, 0, 0, 0},
					{0, cos_x, -sin_x, 0},
					{0, sin_x, cos_x, 0},
					{0, 0, 0, 1}
				}};
			matX = mat4(std::move(matXraw));
		}
		{
			mat4d matYraw =
				{{
					{cos_y, 0, sin_y, 0},
					{0, 1, 0, 0},
					{-sin_y, 0, cos_y, 0},
					{0, 0, 0, 1}
				}};
			matY = mat4(std::move(matYraw));
		}
		{
			mat4d matZraw =
				{{
					{cos_z, -sin_z, 0, 0},
					{sin_z, cos_z, 0, 0},
					{0, 0, 1, 0},
					{0, 0, 0, 1}
				}};
			matZ = mat4(std::move(matZraw));
		}
	}
	// clang-format on

	return matZ * matY * matX;
}

mat4 mat4::makeRotation(const quaternion& q)
{
	mat4 ret{};

	ret[0][0] = 2 * (q._w * q._w + q._x * q._x) - 1;
	ret[0][1] = 2 * (q._x * q._y + q._w * q._z);
	ret[0][2] = 2 * (q._x * q._z + q._w * q._y);

	ret[1][0] = 2 * (q._x * q._y + q._w * q._z);
	ret[1][1] = 2 * (q._w * q._w + q._y * q._y) - 1;
	ret[1][2] = 2 * (q._y * q._z + q._w * q._x);

	ret[2][0] = 2 * (q._x * q._z + q._w * q._y);
	ret[2][1] = 2 * (q._y * q._z + q._w * q._x);
	ret[2][2] = 2 * (q._w * q._w + q._z * q._z) - 1;

	ret[3][3] = 1;
	return ret;
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
	const float tanHalfFov = std::tan(fov / 2.F);

	mat4 ret;
	ret[0][0] = 1.F / (aspect * tanHalfFov);
	ret[1][1] = ret[0][0] * -aspect;
	ret[2][2] = (far + near) / (far - near);
	ret[2][3] = 1;
	ret[3][2] = -(far * near) / (far - near);
	ret[3][3] = 1;

	return ret;
}

mat4 operator*(const mat4& a, const mat4& b)
{
	constexpr size_t N = 4;

	mat4 ret;
	for (size_t i = 0; i < N; i++)
	{
		for (size_t j = 0; j < N; j++)
		{
			float num = 0;
			for (size_t k = 0; k < N; k++)
			{
				num += a[i][k] * b[k][j];
			}
			ret[i][j] = num;
		}
	}

	return ret;
}

mat4 operator*(const mat4& o, const float val)
{
	mat4 ret;
	ret[0][0] = o[0][0] * val;
	ret[0][1] = o[0][1] * val;
	ret[0][2] = o[0][2] * val;
	ret[0][3] = o[0][3] * val;

	ret[1][0] = o[1][0] * val;
	ret[1][1] = o[1][1] * val;
	ret[1][2] = o[1][2] * val;
	ret[1][3] = o[1][3] * val;

	ret[2][0] = o[2][0] * val;
	ret[2][1] = o[2][1] * val;
	ret[2][2] = o[2][2] * val;
	ret[2][3] = o[2][3] * val;

	ret[3][0] = o[3][0] * val;
	ret[3][1] = o[3][1] * val;
	ret[3][2] = o[3][2] * val;
	ret[3][3] = o[3][3] * val;

	return ret;
}

mat4 mat4::makeInverse(const mat4& mat)
{
	const float* m = &mat._mat[0][0];

	mat4d invMat{};
	float* inv = &invMat[0][0];

	inv[0] = m[5] * m[10] * m[15] -
			 m[5] * m[11] * m[14] -
			 m[9] * m[6] * m[15] +
			 m[9] * m[7] * m[14] +
			 m[13] * m[6] * m[11] -
			 m[13] * m[7] * m[10];
	inv[4] = -m[4] * m[10] * m[15] +
			 m[4] * m[11] * m[14] +
			 m[8] * m[6] * m[15] -
			 m[8] * m[7] * m[14] -
			 m[12] * m[6] * m[11] +
			 m[12] * m[7] * m[10];
	inv[8] = m[4] * m[9] * m[15] -
			 m[4] * m[11] * m[13] -
			 m[8] * m[5] * m[15] +
			 m[8] * m[7] * m[13] +
			 m[12] * m[5] * m[11] -
			 m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] * m[14] +
			  m[4] * m[10] * m[13] +
			  m[8] * m[5] * m[14] -
			  m[8] * m[6] * m[13] -
			  m[12] * m[5] * m[10] +
			  m[12] * m[6] * m[9];
	inv[1] = -m[1] * m[10] * m[15] +
			 m[1] * m[11] * m[14] +
			 m[9] * m[2] * m[15] -
			 m[9] * m[3] * m[14] -
			 m[13] * m[2] * m[11] +
			 m[13] * m[3] * m[10];
	inv[5] = m[0] * m[10] * m[15] -
			 m[0] * m[11] * m[14] -
			 m[8] * m[2] * m[15] +
			 m[8] * m[3] * m[14] +
			 m[12] * m[2] * m[11] -
			 m[12] * m[3] * m[10];
	inv[9] = -m[0] * m[9] * m[15] +
			 m[0] * m[11] * m[13] +
			 m[8] * m[1] * m[15] -
			 m[8] * m[3] * m[13] -
			 m[12] * m[1] * m[11] +
			 m[12] * m[3] * m[9];
	inv[13] = m[0] * m[9] * m[14] -
			  m[0] * m[10] * m[13] -
			  m[8] * m[1] * m[14] +
			  m[8] * m[2] * m[13] +
			  m[12] * m[1] * m[10] -
			  m[12] * m[2] * m[9];
	inv[2] = m[1] * m[6] * m[15] -
			 m[1] * m[7] * m[14] -
			 m[5] * m[2] * m[15] +
			 m[5] * m[3] * m[14] +
			 m[13] * m[2] * m[7] -
			 m[13] * m[3] * m[6];
	inv[6] = -m[0] * m[6] * m[15] +
			 m[0] * m[7] * m[14] +
			 m[4] * m[2] * m[15] -
			 m[4] * m[3] * m[14] -
			 m[12] * m[2] * m[7] +
			 m[12] * m[3] * m[6];
	inv[10] = m[0] * m[5] * m[15] -
			  m[0] * m[7] * m[13] -
			  m[4] * m[1] * m[15] +
			  m[4] * m[3] * m[13] +
			  m[12] * m[1] * m[7] -
			  m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] +
			  m[0] * m[6] * m[13] +
			  m[4] * m[1] * m[14] -
			  m[4] * m[2] * m[13] -
			  m[12] * m[1] * m[6] +
			  m[12] * m[2] * m[5];
	inv[3] = -m[1] * m[6] * m[11] +
			 m[1] * m[7] * m[10] +
			 m[5] * m[2] * m[11] -
			 m[5] * m[3] * m[10] -
			 m[9] * m[2] * m[7] +
			 m[9] * m[3] * m[6];
	inv[7] = m[0] * m[6] * m[11] -
			 m[0] * m[7] * m[10] -
			 m[4] * m[2] * m[11] +
			 m[4] * m[3] * m[10] +
			 m[8] * m[2] * m[7] -
			 m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] +
			  m[0] * m[7] * m[9] +
			  m[4] * m[1] * m[11] -
			  m[4] * m[3] * m[9] -
			  m[8] * m[1] * m[7] +
			  m[8] * m[3] * m[5];
	inv[15] = m[0] * m[5] * m[10] -
			  m[0] * m[6] * m[9] -
			  m[4] * m[1] * m[10] +
			  m[4] * m[2] * m[9] +
			  m[8] * m[1] * m[6] -
			  m[8] * m[2] * m[5];

	double det;
	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	if (det == 0)
	{
		return mat4::makeIdentity();
	}
	return mat4(invMat) * det;
}