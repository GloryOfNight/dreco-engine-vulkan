#include "rotator.hxx"

#include "mat4.hxx"
#include "vec3.hxx"

#include <cmath>

rotator::rotator(const float pitch, const float yaw, const float roll)
	: _pitch{pitch}
	, _yaw{yaw}
	, _roll{roll}
{
}

vec3 rotator::toForwardVector() const
{
	rotator clampedRot = *this;
	clampedRot.clamp();

	const auto rotRad = clampedRot.toRadians();

	const float sPitch = std::sin(rotRad._pitch);
	const float cPitch = std::cos(rotRad._pitch);
	const float sYaw = std::sin(rotRad._yaw);
	const float cYaw = std::cos(rotRad._yaw);

	return vec3(cPitch * sYaw, sPitch, cPitch * cYaw);
}

vec3 rotator::toRightDirection() const
{
	const rotator splittedRotator{0, _yaw + 90.f, 0.0F};
	return splittedRotator.toForwardVector();
}

rotator rotator::toRadians() const
{
	return rotator{
		static_cast<float>(_pitch * M_PI) / 180.F,
		static_cast<float>(_yaw * M_PI) / 180.F,
		static_cast<float>(_roll * M_PI) / 180.F};
}

void rotator::clamp()
{
	_pitch = fmodf(_pitch, 360.F);
	_yaw = fmodf(_yaw, 360.F);
	_roll = fmodf(_roll, 360.F);
}

rotator operator+(const rotator& first, const rotator& second)
{
	return rotator(first._pitch + second._pitch, first._yaw + second._yaw, first._roll + second._roll);
}

rotator operator-(const rotator& first, const rotator& second)
{
	return rotator(first._pitch - second._pitch, first._yaw - second._yaw, first._roll - second._roll);
}