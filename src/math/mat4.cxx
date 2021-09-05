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
	constexpr size_t N = 4;

	mat4::mat4d c;
	for (size_t i = 0; i < N; i++)
	{
		for (size_t j = 0; j < N; j++)
		{
			float num = 0;
			for (size_t k = 0; k < N; k++)
			{
				num += a._mat[i][k] * b._mat[k][j];
			}
			c[i][j] = num;
		}
	}

	return mat4(c);
}

mat4 operator*(const mat4& mat, const float val)
{
	const auto& m = mat._mat;
	mat4::mat4d newMat;
	newMat[0][0] = m[0][0] * val;
	newMat[0][1] = m[0][1] * val;
	newMat[0][2] = m[0][2] * val;
	newMat[0][3] = m[0][3] * val;

	newMat[1][0] = m[1][0] * val;
	newMat[1][1] = m[1][1] * val;
	newMat[1][2] = m[1][2] * val;
	newMat[1][3] = m[1][3] * val;

	newMat[2][0] = m[2][0] * val;
	newMat[2][1] = m[2][1] * val;
	newMat[2][2] = m[2][2] * val;
	newMat[2][3] = m[2][3] * val;

	newMat[3][0] = m[3][0] * val;
	newMat[3][1] = m[3][1] * val;
	newMat[3][2] = m[3][2] * val;
	newMat[3][3] = m[3][3] * val;

	return mat4(newMat);
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