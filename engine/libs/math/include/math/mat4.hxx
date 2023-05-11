#pragma once

#include "mat.hxx"
#include "quaternion.hxx"
#include "vec3.hxx"

namespace de::math
{
	struct transform;

	struct mat4 : public mat_t<float, 4>
	{
		mat4() = default;
		mat4(std::array<float, 16>&& rawMat);

		static mat4 makeTransform(const transform& t);

		static mat4 makeTranslation(const vec3& vec);

		static mat4 makeRotation(const quaternion& q);

		static mat4 makeScale(const vec3& vec);

		static mat4 makeIdentity();

		static mat4 makeFirstPersonView(const vec3& translation, const quaternion& rotation);

		static mat4 lookAt(const vec3& pos, const vec3& target, const vec3& up);

		static mat4 makeProjection(const float near, const float far, const float aspect, const float fov);

		static mat4 makeInverse(const mat4& mat);

		mat4& extractScale(const vec3& inScale);

		vec3 getTranslation() const;

		vec3 getScale() const;

		quaternion getRotationQ() const;

		const mat4::collumn& operator[](uint8_t index) const
		{
			return _raw[index];
		}
		mat4::collumn& operator[](uint8_t index)
		{
			return _raw[index];
		}
	};

	mat4 operator*(const mat4& a, const mat4& b);

	mat4 operator*(const mat4& mat, const float val);
} // namespace de::math