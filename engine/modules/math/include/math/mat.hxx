#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace de::math
{
	template <typename T, size_t dim>
	struct mat_t
	{
		mat_t() = default;
		mat_t(std::array<std::array<T, dim>, dim>&& rawMat)
			: mat_t()
		{
			memmove(this, rawMat.data(), sizeof(T) * dim * dim);
		}
		mat_t(std::array<T, dim * dim>&& rawMat)
			: mat_t()
		{
			memmove(this, rawMat.data(), sizeof(T) * rawMat.size());
		}

		struct collumn
		{
			std::array<T, dim> _c{};
			const T& operator[](uint8_t index) const
			{
				return _c[index];
			}
			T& operator[](uint8_t index)
			{
				return _c[index];
			}

			const std::array<T, dim>& operator*() const
			{
				return _c;
			}
		};

		std::array<collumn, dim> _raw{};
		const collumn& operator[](uint8_t index) const
		{
			return _raw[index];
		}
		collumn& operator[](uint8_t index)
		{
			return _raw[index];
		}

		bool operator==(const mat_t<T, dim>& o) const
		{
			for (uint8_t i = 0; i < dim; ++i)
			{
				for (uint8_t j = 0; j < dim; ++j)
				{
					if ((*this)[i][j] != o[i][j])
						return false;
				}
			}
			return true;
		}
	};
} // namespace de::math