#pragma once

#include "quaternion.hxx"
#include "vectors.hxx"

#include <array>
#include <cstddef>
#include <type_traits>

namespace de::math
{
	struct transform;

	template <typename T, size_t dim>
	struct matrix
	{
		matrix() = default;
		matrix(std::array<std::array<T, dim>, dim>&& rawMat)
			: matrix()
		{
			memmove(this, rawMat.data(), sizeof(T) * dim * dim);
		}
		matrix(std::array<T, dim * dim>&& rawMat)
			: matrix()
		{
			memmove(this, rawMat.data(), sizeof(T) * rawMat.size());
		}

		struct collumn
		{
			std::array<float, dim> _c{};
			const float& operator[](uint8_t index) const
			{
				return _c[index];
			}
			float& operator[](uint8_t index)
			{
				return _c[index];
			}

			const std::array<float, dim>& operator*() const
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

		bool operator==(const matrix<T, dim>& o) const
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

	struct DRECO_API mat4 : public matrix<float, 4>
	{
		mat4() = default;
		mat4(std::array<float, 16>&& rawMat);

		static constexpr float size() noexcept;

		static mat4 makeTransform(const transform& t);

		static mat4 makeTranslation(const vec3& vec);

		static mat4 makeRotation(const quaternion& q);

		static mat4 makeScale(const vec3& vec);

		static mat4 makeIdentity();

		static mat4 makeFirstPersonView(const vec3& translation, const quaternion& rotation);

		static mat4 lookAt(const vec3& pos, const vec3& target, const vec3& up);

		static mat4 makeProjection(const float near, const float far, const float aspect, const float fov);

		static mat4 makeInverse(const mat4& mat);

		mat4& extractScale(const vec3& inScale);

		vec3 getTranslation() const;

		vec3 getScale() const;

		quaternion getRotationQ() const;

		const mat4::collumn& operator[](uint8_t index) const
		{
			return _raw[index];
		}
		mat4::collumn& operator[](uint8_t index)
		{
			return _raw[index];
		}
	};

	mat4 operator*(const mat4& a, const mat4& b);

	mat4 operator*(const mat4& mat, const float val);
} // namespace de::math