#pragma once
#include <array>
#include <type_traits>

namespace de::math
{
	template <typename T>
	struct vec2t
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

		bool operator==(const vec2t<T>& other) const
		{
			return _x == other._x && _y == other._y;
		}
	};

	using vec2i = vec2t<int32_t>;
	using vec2d = vec2t<double>;
	using vec2f = vec2t<float>;
	using vec2 = vec2f;
} // namespace de::math
