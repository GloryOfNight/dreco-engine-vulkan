#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace de::math
{
	template <typename T>
	struct vec4_t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec4_t() = default;
		explicit vec4_t(T a, T b, T c, T d)
			: _x{a}
			, _y{b}
			, _z{c}
			, _w{d}
		{
		}

		template <typename K>
		static vec4_t narrow_construct(K a, K b, K c, K d)
		{
			return vec4_t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c), static_cast<T>(d));
		}

		union
		{
			alignas(4) T _x{}, _r;
		};
		union
		{
			alignas(4) T _y{}, _g;
		};
		union
		{
			alignas(4) T _z{}, _b;
		};
		union
		{
			alignas(4) T _w{}, _a;
		};

		vec4_t operator+(const vec4_t<T>& other)
		{
			return vec4_t{this->_x + other._x, this->_y + other._y, this->_z + other._z, this->_w + other._w};
		}
		vec4_t& operator+=(const vec4_t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec4_t operator-(const vec4_t<T>& other) const
		{
			return vec4_t{this->_x - other._x, this->_y - other._y, this->_z - other._z, this->_w - other._w};
		}
		vec4_t& operator-=(const vec4_t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec4_t operator*(const T other) const
		{
			return vec4_t{this->_x * other, this->_y * other, this->_z * other, this->_w * other};
		}
		vec4_t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec4_t operator*(const vec4_t<T>& other) const
		{
			return vec4_t{this->_x * other._x, this->_y * other._y, this->_z * other._z, this->_w * other._w};
		}
		vec4_t& operator*=(const vec4_t<T>& other)
		{
			*this = *this * other;
			return *this;
		}
	};

	using bvec4 = vec4_t<bool>;
	using ivec4 = vec4_t<int32_t>;
	using uvec4 = vec4_t<int32_t>;
	using dvec4 = vec4_t<double>;
	using vec4 = vec4_t<float>;
} // namespace de::math