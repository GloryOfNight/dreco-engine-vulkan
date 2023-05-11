#pragma once
#include "constants.hxx"
#include "vec3.hxx"

#include <type_traits>

namespace de::math
{
	template <typename T>
	struct euler_t
	{
		euler_t() = default;
		euler_t(const T x, const T y, const T z)
			: _x{x}
			, _y{y}
			, _z{z}
		{
		}

		vec3 toForwardVector() const
		{
			const auto sPitch = std::sin(_pitch);
			const auto cPitch = std::cos(_pitch);
			const auto sYaw = std::sin(_yaw);
			const auto cYaw = std::cos(_yaw);
			return vec3(cPitch * sYaw, sPitch, cPitch * cYaw);
		}

		vec3 toRightDirection() const
		{
			return euler_t(static_cast<T>(0), _yaw + 0.5 * Pi, static_cast<T>(0)).toForwardVector();
		}

		void clamp()
		{
			_x = fmodf(_x, 2 * Pi);
			_y = fmodf(_y, 2 * Pi);
			_z = fmodf(_z, 2 * Pi);
		}

		void max(const T pitch, const T yaw, const T roll)
		{
			if (_x > pitch)
				_x = pitch;
			if (_y > yaw)
				_y = yaw;
			if (_z > roll)
				_z = roll;
		};
		void min(const T pitch, const T yaw, const T roll)
		{
			if (_x < pitch)
				_x = pitch;
			if (_y < yaw)
				_y = yaw;
			if (_z < roll)
				_z = roll;
		};

		euler_t operator+(const euler_t& other) const
		{
			return euler_t{_x + other._x, _y + other._y, _z + other._z};
		}

		euler_t& operator+=(const euler_t& other) const
		{
			return *this = *this + other;
		}

		euler_t operator*(const float value) const
		{
			return euler_t{_x * value, _y * value, _z * value};
		}

		union
		{
			T _pitch, _x;
		};

		union
		{
			T _yaw, _y;
		};

		union
		{
			T _roll, _z;
		};
	};

	using euler = euler_t<float>;
} // namespace de::math