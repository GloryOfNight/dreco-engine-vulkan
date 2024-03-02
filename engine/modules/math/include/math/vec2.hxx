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
			alignas(4) T _x{}, _u;
		};
		union
		{
			alignas(4) T _y{}, _v;
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

	using bvec2 = vec2_t<bool>;
	using ivec2 = vec2_t<int32_t>;
	using uvec2 = vec2_t<int32_t>;
	using dvec2 = vec2_t<double>;
	using vec2 = vec2_t<float>;
} // namespace de::math
