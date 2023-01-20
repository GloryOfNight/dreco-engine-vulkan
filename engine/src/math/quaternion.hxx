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

		bool is_normalized() const
		{
			return (_x * _x + _y * _y + _z * _z + _w * _w) == static_cast<T>(1);
		}

		void normalize()
		{
			constexpr float EPSILON = 1e-6;
			const float magnitude = std::sqrtf(_x * _x + _y * _y + _z * _z + _w * _w);
			if (std::isnan(magnitude) || magnitude == static_cast<T>(0))
			{
				*this = identity();
				return;
			}

			_x /= magnitude;
			if (std::fabs(_x) < EPSILON)
				_x = 0.f;

			_y /= magnitude;
			if (std::fabs(_y) < EPSILON)
				_y = 0.f;

			_z /= magnitude;
			if (std::fabs(_z) < EPSILON)
				_z = 0.f;

			_w /= magnitude;
			if (std::fabs(_w) < EPSILON)
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