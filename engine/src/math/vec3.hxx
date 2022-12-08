#pragma once
#include "vec.hxx"

#include <type_traits>

template <typename T>
struct DRECO_API vec3t
{
	static_assert(std::is_trivial<T>::value, "T must be trivial");

	vec3t() = default;
	explicit vec3t(T a, T b, T c)
		: _x{a}
		, _y{b}
		, _z{c}
	{
	}

	template<typename K>
	static vec3t narrow_construct(K a, K b, K c) 
	{
		return vec3t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c));
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