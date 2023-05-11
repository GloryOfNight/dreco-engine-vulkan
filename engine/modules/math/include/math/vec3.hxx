#pragma once
#include <array>
#include <cstddef>

namespace de::math
{
	template <typename T>
	struct vec3_t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec3_t() = default;
		explicit vec3_t(T a, T b, T c)
			: _x{a}
			, _y{b}
			, _z{c}
		{
		}

		explicit vec3_t(const std::array<T, 4>& matCollumn)
			: _x{matCollumn[0]}
			, _y{matCollumn[1]}
			, _z{matCollumn[2]}
		{
		}

		template <typename K>
		static vec3_t narrow_construct(K a, K b, K c)
		{
			return vec3_t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c));
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

		static vec3_t normalize(const vec3_t& v)
		{
			const auto len = length(v);
			return len == static_cast<T>(0) ? vec3_t() : vec3_t(v._x / len, v._y / len, v._z / len);
		}

		static vec3_t cross(const vec3_t& f, const vec3_t& s)
		{
			return vec3_t(f._y * s._z - f._z * s._y,
				f._z * s._x - f._x * s._z,
				f._x * s._y - f._y * s._x);
		}

		static float dot(const vec3_t<T>& first, const vec3_t<T>& second)
		{
			return (first._x * second._x) + (first._y * second._y) + (first._z * second._z);
		}

		static float length(const vec3_t<T>& value)
		{
			return std::sqrt(value._x * value._x + value._y * value._y + value._z * value._z);
		}

		vec3_t operator+(const vec3_t<T>& other) const
		{
			return vec3_t{_x + other._x, _y + other._y, _z + other._z};
		}
		vec3_t& operator+=(const vec3_t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec3_t operator-() const
		{
			return vec3_t{-_x, -_y, -_z};
		}
		vec3_t operator-(const vec3_t<T>& other) const
		{
			return vec3_t{_x - other._x, _y - other._y, _z - other._z};
		}
		vec3_t& operator-=(const vec3_t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec3_t operator*(const T other) const
		{
			return vec3_t{_x * other, _y * other, _z * other};
		}
		vec3_t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec3_t operator*(const vec3_t<T>& other) const
		{
			return vec3_t{_x * other._x, _y * other._y, _z * other._z};
		}
		vec3_t& operator*=(const vec3_t<T>& other)
		{
			*this = *this * other;
			return *this;
		}
	};

	using vec3i = vec3_t<int32_t>;
	using vec3d = vec3_t<double>;
	using vec3f = vec3_t<float>;
	using vec3 = vec3f;
} // namespace de::math