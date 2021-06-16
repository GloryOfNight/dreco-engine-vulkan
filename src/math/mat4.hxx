#pragma once
#include <array>
#include <cstddef>

struct vec3;
struct rotator;
struct transform;

enum matAxis
{
	X,
	Y,
	Z
};

struct mat4
{
	typedef std::array<std::array<float, 4>, 4> mat4d;

	mat4();

	explicit mat4(const mat4d& _mat);

	static constexpr float size() noexcept;

	static mat4 makeTransform(const transform& t);

	static mat4 makeTranslation(const vec3& vec);

	static mat4 makeRotation(const rotator& rot);

	static mat4 makeScale(const vec3& vec);

	static mat4 makeIdentity();

	static mat4 makeProjection(const float near, const float far, const float aspect, const float fov);

	vec3 getAxis(const matAxis axis) const;

	mat4d _mat;
};

mat4 operator*(const mat4& a, const mat4& b);
