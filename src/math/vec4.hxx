#pragma once
#include "vec.hxx"

#include <type_traits>

template <typename T>
struct DRECO_API vec4t
{
	vec4t() = default;
	vec4t(const T& a, const T& b, const T& c, const T& d)
		: _x{a}
		, _y{b}
		, _z{c}
		, _w{d}
	{
	}

	union
	{
		T _x, _r;
	};
	union
	{
		T _y, _g;
	};
	union
	{
		T _z, _b;
	};
	union
	{
		T _w, _a;
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