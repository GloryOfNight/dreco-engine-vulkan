#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace de::math
{
	template <typename T>
	struct vec2_t
	{
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		vec2_t() = default;
		explicit vec2_t(T a, T b)
			: _x{a}
			, _y{b}
		{
		}

		template <typename K>
		static vec2_t narrow_construct(K a, K b)
		{
			return vec2_t(static_cast<T>(a), static_cast<T>(b));
		}

		union
		{
			T _x{}, _u;
		};
		union
		{
			T _y{}, _v;
		};

		vec2_t operator+(const vec2_t<T>& other)
		{
			return vec2_t{this->_x + other._x, this->_y + other._y};
		}
		vec2_t& operator+=(const vec2_t<T>& other)
		{
			*this = *this + other;
			return *this;
		}

		vec2_t operator-(const vec2_t<T>& other) const
		{
			return vec2_t{this->_x - other._x, this->_y - other._y};
		}
		vec2_t& operator-=(const vec2_t<T>& other)
		{
			*this = *this - other;
			return *this;
		}

		vec2_t operator*(const T other) const
		{
			return vec2_t{this->_x * other, this->_y * other};
		}
		vec2_t& operator*=(const T other)
		{
			*this = *this * other;
			return *this;
		}

		vec2_t operator*(const vec2_t<T>& other) const
		{
			return vec2_t{this->_x * other._x, this->_y * other._y};
		}
		vec2_t& operator*=(const vec2_t<T>& other)
		{
			*this = *this * other;
			return *this;
		}

		bool operator==(const vec2_t<T>& other) const
		{
			return _x == other._x && _y == other._y;
		}
	};

	using vec2i = vec2_t<int32_t>;
	using vec2d = vec2_t<double>;
	using vec2f = vec2_t<float>;
	using vec2 = vec2f;
} // namespace de::math
