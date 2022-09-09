#pragma once
#include "vec.hxx"

template <typename T>
struct DRECO_API vec2t
{
	vec2t() = default;
	vec2t(const T& a, const T& b)
		: _x{a}
		, _y{b}
	{
	}

	union
	{
		T _x, _u;
	};
	union
	{
		T _y, _v;
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