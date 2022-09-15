#pragma once
#include "vec.hxx"

#include <array>
#include <cstddef>

struct rotatorRad;
struct transform;
struct quaternion;

template <typename T, uint8_t dim>
struct matrix
{
	struct collumn
	{
		float _c[dim]{};
		const float& operator[](uint8_t index) const
		{
			return _c[index];
		}
		float& operator[](uint8_t index)
		{
			return _c[index];
		}
	};

	collumn _rc[dim]{};
	const collumn& operator[](uint8_t index) const
	{
		return _rc[index];
	}
	collumn& operator[](uint8_t index)
	{
		return _rc[index];
	}
};

struct DRECO_API mat4
{
	using mat4d = matrix<float, 4>;

	mat4() = default;

	explicit mat4(const mat4d& mat);

	explicit mat4(mat4d&& mat);

	static constexpr float size() noexcept;

	static mat4 makeTransform(const transform& t);

	static mat4 makeTranslation(const vec3& vec);

	static mat4 makeRotation(const rotatorRad& rot);

	static mat4 makeRotationQ(const quaternion& q);

	static mat4 makeScale(const vec3& vec);

	static mat4 makeIdentity();

	static mat4 makeProjection(const float near, const float far, const float aspect, const float fov);

	static mat4 makeInverse(const mat4& mat);

	mat4d _mat{};

	const mat4d::collumn& operator[](uint8_t index) const
	{
		return _mat[index];
	}
	mat4d::collumn& operator[](uint8_t index)
	{
		return _mat[index];
	}
};

mat4 operator*(const mat4& a, const mat4& b);

mat4 operator*(const mat4& mat, const float val);
