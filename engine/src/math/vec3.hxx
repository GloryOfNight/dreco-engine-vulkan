#pragma once
#include "vec.hxx"

#include <type_traits>

template <typename T>
struct DRECO_API vec3t
{
	vec3t() = default;
	vec3t(const T& a, const T& b, const T& c)
		: _x{a}
		, _y{b}
		, _z{c}
	{
	}

	template <typename Arr>
	vec3t(Arr&& arr)
		: _x{static_cast<T>(arr[0])}
		, _y{static_cast<T>(arr[1])}
		, _z{static_cast<T>(arr[2])}
	{
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

	static float dot(const vec3t<T>& first, const vec3t<T>& second)
	{
		return (first._x * second._x) + (first._y * second._y) + (first._z * second._z);
	}

	vec3t operator+(const vec3t<T>& other)
	{
		return vec3t{this->_x + other._x, this->_y + other._y, this->_z + other._z};
	}
	vec3t& operator+=(const vec3t<T>& other)
	{
		*this = *this + other;
		return *this;
	}

	vec3t operator-(const vec3t<T>& other) const
	{
		return vec3t{this->_x - other._x, this->_y - other._y, this->_z - other._z};
	}
	vec3t& operator-=(const vec3t<T>& other)
	{
		*this = *this - other;
		return *this;
	}

	vec3t operator*(const T other) const
	{
		return vec3t{this->_x * other, this->_y * other, this->_z * other};
	}
	vec3t& operator*=(const T other)
	{
		*this = *this * other;
		return *this;
	}

	vec3t operator*(const vec3t<T>& other) const
	{
		return vec3t{this->_x * other._x, this->_y * other._y, this->_z * other._z};
	}
	vec3t& operator*=(const vec3t<T>& other)
	{
		*this = *this * other;
		return *this;
	}
};