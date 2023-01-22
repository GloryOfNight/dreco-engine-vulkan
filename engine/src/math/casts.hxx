#pragma once

#include "euler.hxx"
#include "mat4.hxx"
#include "quaternion.hxx"
#include "transform.hxx"
#include "vec3.hxx"

#include <cmath>

namespace de::math
{
	template <typename T>
	constexpr T deg_to_rad(const T value)
	{
		static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
		return static_cast<T>(value * static_cast<T>(M_PI)) / static_cast<T>(180);
	}

	template <typename T>
	constexpr T rad_to_deg(const T value)
	{
		static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
		return static_cast<T>(value * static_cast<T>(180)) / static_cast<T>(M_PI);
	}

	inline euler euler_cast(const quaternion& q)
	{
		return euler(quaternion::pitch(q), quaternion::yaw(q), quaternion::roll(q));
	}

	inline quaternion quat_cast(const euler& rotation)
	{
		const auto r = rotation * 0.5f;

		const auto cx = cosf(r._x);
		const auto cy = cosf(r._y);
		const auto cz = cosf(r._z);

		const auto sx = sinf(r._x);
		const auto sy = sinf(r._y);
		const auto sz = sinf(r._z);

		quaternion q;
		q._w = cx * cy * cz + sx * sy * sz;
		q._x = sx * cy * cz - cx * sy * sz;
		q._y = cx * sy * cz + sx * cy * sz;
		q._z = cx * cy * sz - sx * sy * cz;
		q.normalize();
		return q;
	}

	template <typename T = transform>
	T transform_cast(const mat4& m)
	{
		transform t;
		mat4 mCopy = m;
		t._scale = mCopy.getScale();
		mCopy.extractScale(t._scale);

		t._rotation = euler_cast(mCopy.getRotationQ());

		// make invert of rotation matrix and "unrotate" the translation
		mCopy[3][0] = mCopy[3][1] = mCopy[3][2] = 0.f;
		mCopy = m * mat4::makeInverse(mCopy);

		t._translation = mCopy.getTranslation();
		return t;
	}
} // namespace de::math
