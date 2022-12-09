#pragma once
#include "vectors.hxx"

#include <type_traits>

namespace de::math
{
	template <typename T>
	struct DRECO_API vec4t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec4t() = default;
		explicit vec4t(T a, T b, T c, T d)
			: _x{a}
			, _y{b}
			, _z{c}
			, _w{d}
		{
		}

		template <typename K>
		static vec4t narrow_construct(K a, K b, K c, K d)
		{
			return vec4t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c), static_cast<T>(d));
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
		union
		{
			T _w{}, _a;
		};

		vec4t operator+(const vec4t<T>& other)
		{
			return vec4t{this->_x + other._x, this->_y + other._y, this->_z + other._z, this->_w + other._w};
		}
		vec4t& operator+=(const vec4t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec4t operator-(const vec4t<T>& other) const
		{
			return vec4t{this->_x - other._x, this->_y - other._y, this->_z - other._z, this->_w - other._w};
		}
		vec4t& operator-=(const vec4t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec4t operator*(const T other) const
		{
			return vec4t{this->_x * other, this->_y * other, this->_z * other, this->_w * other};
		}
		vec4t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec4t operator*(const vec4t<T>& other) const
		{
			return vec4t{this->_x * other._x, this->_y * other._y, this->_z * other._z, this->_w * other._w};
		}
		vec4t& operator*=(const vec4t<T>& other)
		{
			*this = *this * other;
			return *this;
		}
	};
} // namespace de::math