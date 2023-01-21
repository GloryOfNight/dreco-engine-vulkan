#include "mat4.hxx"

#include "casts.hxx"
#include "euler.hxx"
#include "quaternion.hxx"
#include "transform.hxx"
#include "vec3.hxx"

#include <cmath>
#include <cstring>

de::math::mat4::mat4(std::array<float, 16>&& rawMat)
	: matrix(std::move(rawMat))
{
}

constexpr float de::math::mat4::size() noexcept
{
	return 16;
}

de::math::mat4 de::math::mat4::makeTransform(const transform& t)
{
	return makeTranslation(t._translation) * makeRotation(quat_cast(t._rotation)) * makeScale(t._scale);
}

de::math::mat4 de::math::mat4::makeTranslation(const vec3& vec)
{
	// clang-format off
	std::array<float, 16> rawMat =
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			vec._x, vec._y, vec._z, 1
		};
	// clang-format on
	return mat4(std::move(rawMat));
}

de::math::mat4 de::math::mat4::makeRotation(const quaternion& q)
{
	mat4 out{};

	out[0][0] = 1.0f - 2.0f * q._y * q._y - 2.0f * q._z * q._z;
	out[0][1] = 2.0f * q._x * q._y - 2.0f * q._z * q._w;
	out[0][2] = 2.0f * q._x * q._z + 2.0f * q._y * q._w;

	out[1][0] = 2.0f * q._x * q._y + 2.0f * q._z * q._w;
	out[1][1] = 1.0f - 2.0f * q._x * q._x - 2.0f * q._z * q._z;
	out[1][2] = 2.0f * q._y * q._z - 2.0f * q._x * q._w;

	out[2][0] = 2.0f * q._x * q._z - 2.0f * q._y * q._w;
	out[2][1] = 2.0f * q._y * q._z + 2.0f * q._x * q._w;
	out[2][2] = 1.0f - 2.0f * q._x * q._x - 2.0f * q._y * q._y;
	out[3][3] = 1.f;
	return out;
}

de::math::mat4 de::math::mat4::makeScale(const vec3& vec)
{
	// clang-format off
	std::array<float, 16> rawMat =
		{
			vec._x, 0, 0, 0,
			0, vec._y, 0, 0,
			0, 0, vec._z, 0,
			0, 0, 0, 1
		};
	// clang-format on
	return mat4(std::move(rawMat));
}

de::math::mat4 de::math::mat4::makeIdentity()
{
	// clang-format off
	std::array<float, 16> rawMat = 
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
	// clang-format on
	return mat4(std::move(rawMat));
}

de::math::mat4 de::math::mat4::makeFirstPersonView(const vec3& translation, const quaternion& rotation)
{
	return makeTranslation(-translation) * makeRotation(rotation) * makeScale(vec3(1.f, -1.f, 1.f));
}

de::math::mat4 de::math::mat4::lookAt(const vec3& pos, const vec3& target, const vec3& up)
{
	// BUG: when looking up or down, generates invalid matrix for camera to view
	mat4 out;
	const auto z_axis = vec3::normalize(vec3(target._x - pos._x, target._y - pos._y, target._z - pos._z));
	
	const auto x_axis = vec3::normalize(vec3::cross(z_axis, up));
	const auto y_axis = vec3::cross(x_axis, z_axis);

	out[0][0] = x_axis._x;
	out[0][1] = y_axis._x;
	out[0][2] = -z_axis._x;
	out[0][3] = 0;

	out[1][0] = x_axis._y;
	out[1][1] = y_axis._y;
	out[1][2] = -z_axis._y;
	out[1][3] = 0;

	out[2][0] = x_axis._z;
	out[2][1] = y_axis._z;
	out[2][2] = -z_axis._z;
	out[2][3] = 0;

	out[3][0] = -vec3::dot(x_axis, pos);
	out[3][1] = -vec3::dot(y_axis, pos);
	out[3][2] = vec3::dot(z_axis, pos);
	out[3][3] = 1.0f;

	return out;
}

de::math::mat4 de::math::mat4::makeProjection(const float near, const float far, const float aspect, const float fov)
{
	const float tanHalfFov = std::tan(fov / 2.F);
	mat4 out;
	out[0][0] = 1.0f / (aspect * tanHalfFov);
	out[1][1] = 1.0f / tanHalfFov;
	out[2][2] = -((far + near) / (far - near));
	out[2][3] = -1.0f;
	out[3][2] = -((2.0f * far * near) / (far - near));
	out[3][3] = 1.f;
	return out;
}

de::math::mat4 de::math::operator*(const de::math::mat4& a, const de::math::mat4& b)
{
	constexpr size_t N = 4;

	mat4 ret{};
	for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
			for (size_t k = 0; k < N; ++k)
				ret[i][j] += a[i][k] * b[k][j];
	return ret;
}

de::math::mat4 de::math::operator*(const de::math::mat4& o, const float val)
{
	de::math::mat4 ret;
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

de::math::mat4 de::math::mat4::makeInverse(const mat4& mat)
{
	const float* m = &mat[0][0];

	mat4 invMat{};
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

	const auto det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	if (det == 0.f)
	{
		return mat4::makeIdentity();
	}
	return mat4(invMat) * det;
}

de::math::mat4& de::math::mat4::extractScale(const vec3& inScale)
{
	if (inScale._x)
	{
		const float invScale = 1.f / inScale._x;
		(*this)[0][0] *= invScale;
		(*this)[0][1] *= invScale;
		(*this)[0][2] *= invScale;
	}
	if (inScale._y)
	{
		const float invScale = 1.f / inScale._y;
		(*this)[1][0] *= invScale;
		(*this)[1][1] *= invScale;
		(*this)[1][2] *= invScale;
	}
	if (inScale._z)
	{
		const float invScale = 1.f / inScale._z;
		(*this)[2][0] *= invScale;
		(*this)[2][1] *= invScale;
		(*this)[2][2] *= invScale;
	}
	return *this;
}

de::math::vec3 de::math::mat4::getTranslation() const
{
	return vec3((*this)[3][0], (*this)[3][1], (*this)[3][2]);
}

de::math::vec3 de::math::mat4::getScale() const
{
	float x = vec3::length(vec3(*(*this)[0]));
	float y = vec3::length(vec3(*(*this)[1]));
	float z = vec3::length(vec3(*(*this)[2]));
	return vec3(x, y, z);
}

de::math::quaternion de::math::mat4::getRotationQ() const
{
	de::math::quaternion q;

	const float trace = (*this)[0][0] + (*this)[1][1] + (*this)[2][2];
	float s;
	if (trace > 0.0f)
	{
		const float invS = 1.f / std::sqrt(trace + 1.f);
		q._w = 0.5f * (1.f / invS);
		s = 0.5f * invS;

		q._x = ((*this)[1][2] - (*this)[2][1]) * s;
		q._y = ((*this)[2][0] - (*this)[0][2]) * s;
		q._z = ((*this)[0][1] - (*this)[1][0]) * s;
	}
	else // diagonal is negative
	{
		uint8_t i = 0;

		if ((*this)[1][1] > (*this)[0][0])
			i = 1;

		if ((*this)[2][2] > (*this)[i][i])
			i = 2;

		constexpr uint8_t nxt[3] = {1, 2, 0};
		const uint8_t j = nxt[i];
		const uint8_t k = nxt[j];

		s = (*this)[i][i] - (*this)[j][j] - (*this)[k][k] + 1.0f;

		const float invS = 1.f / std::sqrt(s);

		float qt[4]{};
		qt[i] = 0.5f * (1.f / invS);

		s = 0.5f * invS;

		qt[3] = ((*this)[j][k] - (*this)[k][j]) * s;
		qt[j] = ((*this)[i][j] + (*this)[j][i]) * s;
		qt[k] = ((*this)[i][k] + (*this)[k][i]) * s;

		q._x = qt[0];
		q._y = qt[1];
		q._z = qt[2];
		q._w = qt[3];
		q.normalize();
	}
	return q;
}