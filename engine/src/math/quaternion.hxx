#pragma once

#include <type_traits>

template <typename T>
struct quaternion_t
{
	static_assert(std::is_trivial<T>::value, "T must be trivial");

	quaternion_t() = default;
	explicit quaternion_t(T x, T y, T z, T w)
		: _x{x}
		, _y{y}
		, _z{z}
		, _w{w}
	{
	}

	template <typename K>
	static quaternion_t narrow_construct(K a, K b, K c, K d)
	{
		return quaternion_t(static_cast<T>(a), static_cast<T>(b), static_cast<T>(c), static_cast<T>(d));
	}

	T _x{};
	T _y{};
	T _z{};
	T _w{};
};

using quaterniond = quaternion_t<double>;
using quaternionf = quaternion_t<float>;
using quaternion = quaternionf;