#pragma once
#include "math.hxx"
#include "vectors.hxx"

namespace de::math
{
	template <typename T>
	struct rotator3t
	{
		rotator3t() = default;
		rotator3t(const T pitch, const T yaw, const T roll)
			: _pitch{pitch}
			, _yaw{yaw}
			, _roll{roll}
		{
		}

		vec3 toForwardVector() const
		{
			const auto rPitch = math::degreesToRadians(_pitch);
			const auto rYaw = math::degreesToRadians(_yaw);

			const auto sPitch = std::sin(rPitch);
			const auto cPitch = std::cos(rPitch);
			const auto sYaw = std::sin(rYaw);
			const auto cYaw = std::cos(rYaw);
			return vec3(cPitch * sYaw, sPitch, cPitch * cYaw);
		}

		vec3 toRightDirection() const
		{
			return rotator3t(static_cast<T>(0), _yaw + static_cast<T>(90), static_cast<T>(0)).toForwardVector();
		}

		vec3 toRadians() const
		{
			return vec3(math::degreesToRadians(_pitch), math::degreesToRadians(_yaw), math::degreesToRadians(_roll));
		}

		void clamp()
		{
			_pitch = fmodf(_pitch, 360.F);
			_yaw = fmodf(_yaw, 360.F);
			_roll = fmodf(_roll, 360.F);
		}

		void max(const T pitch, const T yaw, const T roll)
		{
			if (_pitch > pitch)
				_pitch = pitch;
			if (_yaw > yaw)
				_yaw = yaw;
			if (_roll > roll)
				_roll = roll;
		};
		void min(const T pitch, const T yaw, const T roll)
		{
			if (_pitch < pitch)
				_pitch = pitch;
			if (_yaw < yaw)
				_yaw = yaw;
			if (_roll < roll)
				_roll = roll;
		};

		T _pitch;

		T _yaw;

		T _roll;
	};

	using rotator = rotator3t<float>;
} // namespace de::math