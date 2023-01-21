#pragma once
#include "vectors.hxx"

#include <array>
#include <type_traits>

namespace de::math
{
	template <typename T>
	struct DRECO_API vec3t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec3t() = default;
		explicit vec3t(T a, T b, T c)
			: _x{a}
			, _y{b}
			, _z{c}
		{
		}

		explicit vec3t(const std::array<T, 4>& matCollumn)
			: _x{matCollumn[0]}
			, _y{matCollumn[1]}
			, _z{matCollumn[2]}
		{
		}

		template <typename K>
		static vec3t narrow_construct(K a, K b, K c)
		{
			return vec3t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c));
		}

		union
		{
			T _x{}, _r;
		};
		union
		{
			T _y{}, _g;
		};
		union
		{
			T _z{}, _b;
		};

		static vec3t normalize(const vec3t& v)
		{
			const auto len = length(v);
			return len == static_cast<T>(0) ? vec3t() : vec3t(v._x / len, v._y / len, v._z / len);
		}

		static vec3t cross(const vec3t& f, const vec3t& s)
		{
			return vec3t(f._y * s._z - f._z * s._y,
				f._z * s._x - f._x * s._z,
				f._x * s._y - f._y * s._x);
		}

		static float dot(const vec3t<T>& first, const vec3t<T>& second)
		{
			return (first._x * second._x) + (first._y * second._y) + (first._z * second._z);
		}

		static float length(const vec3t<T>& value)
		{
			return std::sqrt(value._x * value._x + value._y * value._y + value._z * value._z);
		}

		vec3t operator+(const vec3t<T>& other) const
		{
			return vec3t{_x + other._x, _y + other._y, _z + other._z};
		}
		vec3t& operator+=(const vec3t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec3t operator-() const
		{
			return vec3t{-_x, -_y, -_z};
		}
		vec3t operator-(const vec3t<T>& other) const
		{
			return vec3t{_x - other._x, _y - other._y, _z - other._z};
		}
		vec3t& operator-=(const vec3t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec3t operator*(const T other) const
		{
			return vec3t{_x * other, _y * other, _z * other};
		}
		vec3t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec3t operator*(const vec3t<T>& other) const
		{
			return vec3t{_x * other._x, _y * other._y, _z * other._z};
		}
		vec3t& operator*=(const vec3t<T>& other)
		{
			*this = *this * other;
			return *this;
		}
	};
} // namespace de::math