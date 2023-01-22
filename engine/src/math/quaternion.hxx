#pragma once

#include "vec3.hxx"

#include <cmath>
#include <type_traits>

namespace de::math
{
	template <typename T>
	struct quaternion_t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		quaternion_t() = default;
		explicit quaternion_t(T x, T y, T z, T w)
			: _x{x}
			, _y{y}
			, _z{z}
			, _w{w}
		{
		}

		template <typename K>
		static quaternion_t narrow_construct(K a, K b, K c, K d)
		{
			return quaternion_t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c), static_cast<T>(d));
		}

		static quaternion_t identity()
		{
			return quaternion_t(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
		}

		static quaternion_t from_axis_angle(vec3 axis, float angle)
		{
			const float half_angle = 0.5f * angle;
			const float s = std::sin(half_angle);
			const float c = std::cos(half_angle);
			return quaternion_t(s * axis._x, s * axis._y, s * axis._z, c);
		}

		static vec3 forwardVector(const quaternion_t& q)
		{
			const auto x = 2.0f * q._x * q._z + 2.0f * q._y * q._w;
			const auto y = 2.0f * q._y * q._z - 2.0f * q._x * q._w;
			const auto z = 1.0f - 2.0f * q._x * q._x - 2.0f * q._y * q._y;
			return vec3(-x, -y, -z);
		}

		static vec3 rightVector(const quaternion_t& q)
		{
			const auto x = 1.0f - 2.0f * q._y * q._y - 2.0f * q._z * q._z;
			const auto y = 2.0f * q._x * q._y + 2.0f * q._z * q._w;
			const auto z = 2.0f * q._x * q._z - 2.0f * q._y * q._w;
			return vec3(x, y, z);
		}

		static vec3 upVector(const quaternion_t& q)
		{
			const auto x = 2.0f * q._x * q._y - 2.0f * q._z * q._w;
			const auto y = 1.0f - 2.0f * q._x * q._x - 2.0f * q._z * q._z;
			const auto z = 2.0f * q._y * q._z + 2.0f * q._x * q._w;
			return vec3(x, y, z);
		}

		static float dot(const quaternion_t& q)
		{
			return (q._x * q._x + q._y * q._y + q._z * q._z + q._w * q._w);
		}

		static T pitch(const quaternion_t& q)
		{
			const auto y = static_cast<T>(2) * (q._y * q._z + q._w * q._x);
			const auto x = q._w * q._w - q._x * q._x - q._y * q._y + q._z * q._z;
			return vec2(x, y) == vec2(0, 0) ? 0 : atan2(y, x);
		}

		static T yaw(const quaternion_t& q)
		{
			auto v = static_cast<T>(-2) * (q._x * q._z - q._w * q._y);
			if (v > static_cast<T>(1))
				v = static_cast<T>(1);
			else if (v < static_cast<T>(-1))
				v = static_cast<T>(-1);
			return asin(v);
		}

		static T roll(const quaternion_t& q)
		{
			const auto y = static_cast<T>(2) * (q._x * q._y + q._w * q._z);
			const auto x = q._w * q._w + q._x * q._x - q._y * q._y - q._z * q._z;
			return vec2(x, y) == vec2(0, 0) ? 0 : atan2(y, x);
		}

		void normalize()
		{
			constexpr float epsilon = 1e-6;
			const float magnitude = sqrtf(_x * _x + _y * _y + _z * _z + _w * _w);
			if (std::isnan(magnitude) || magnitude == static_cast<T>(0))
			{
				*this = identity();
				return;
			}

			_x /= magnitude;
			if (std::fabs(_x) < epsilon)
				_x = 0.f;

			_y /= magnitude;
			if (std::fabs(_y) < epsilon)
				_y = 0.f;

			_z /= magnitude;
			if (std::fabs(_z) < epsilon)
				_z = 0.f;

			_w /= magnitude;
			if (std::fabs(_w) < epsilon)
				_w = 0.f;
		}

		quaternion_t operator*(const quaternion_t& q) const
		{
			quaternion_t out;
			out._x = _x * q._w +
					 _y * q._z -
					 _z * q._y +
					 _w * q._x;

			out._y = -_x * q._z +
					 _y * q._w +
					 _z * q._x +
					 _w * q._y;

			out._z = _x * q._y -
					 _y * q._x +
					 _z * q._w +
					 _w * q._z;

			out._w = -_x * q._x -
					 _y * q._y -
					 _z * q._z +
					 _w * q._w;
			return out;
		}

		T _x{};
		T _y{};
		T _z{};
		T _w{};
	};

	using quaterniond = quaternion_t<double>;
	using quaternionf = quaternion_t<float>;
	using quaternion = quaternionf;
} // namespace de::math