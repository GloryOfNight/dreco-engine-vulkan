#pragma once
#include "vectors.hxx"

#include <type_traits>

namespace de::math
{
	template <typename T>
	struct DRECO_API vec2t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec2t() = default;
		explicit vec2t(T a, T b)
			: _x{a}
			, _y{b}
		{
		}

		template <typename K>
		static vec2t narrow_construct(K a, K b)
		{
			return vec2t(static_cast<T>(a), static_cast<T>(b));
		}

		union
		{
			T _x{}, _u;
		};
		union
		{
			T _y{}, _v;
		};

		vec2t operator+(const vec2t<T>& other)
		{
			return vec2t{this->_x + other._x, this->_y + other._y};
		}
		vec2t& operator+=(const vec2t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec2t operator-(const vec2t<T>& other) const
		{
			return vec2t{this->_x - other._x, this->_y - other._y};
		}
		vec2t& operator-=(const vec2t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec2t operator*(const T other) const
		{
			return vec2t{this->_x * other, this->_y * other};
		}
		vec2t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec2t operator*(const vec2t<T>& other) const
		{
			return vec2t{this->_x * other._x, this->_y * other._y};
		}
		vec2t& operator*=(const vec2t<T>& other)
		{
			*this = *this * other;
			return *this;
		}
	};
} // namespace de::math