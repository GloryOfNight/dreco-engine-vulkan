#pragma once

#include "SDL_stdinc.h"

namespace de::math
{
	template <typename T>
	static constexpr T radiansToDegrees(const T value)
	{
		return static_cast<T>(value * static_cast<T>(180)) / static_cast<T>(M_PI);
	}

	template <typename T>
	static constexpr T degreesToRadians(const T value)
	{
		return static_cast<T>(value * static_cast<T>(M_PI)) / static_cast<T>(180);
	}
} // namespace de::math