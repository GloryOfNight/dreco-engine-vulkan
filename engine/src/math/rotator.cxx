#include "rotator.hxx"

#include "mat4.hxx"
#include "vec3.hxx"

#include <SDL2/SDL_hints.h>
#include <cmath>

vec3 rotatorDeg::toForwardVector() const
{
	return toRadians().toForwardVector();
}

vec3 rotatorDeg::toRightDirection() const
{
	return rotatorDeg(0.F, _yaw + 90.f, 0.0F).toForwardVector();
}

rotatorRad rotatorDeg::toRadians() const
{
	return rotatorRad(static_cast<float>(_pitch * M_PI) / 180.F, static_cast<float>(_yaw * M_PI) / 180.F, static_cast<float>(_roll * M_PI) / 180.F);
}

void rotatorDeg::clamp()
{
	_pitch = fmodf(_pitch, 360.F);
	_yaw = fmodf(_yaw, 360.F);
	_roll = fmodf(_roll, 360.F);
}

vec3 rotatorRad::toForwardVector() const
{
	rotatorRad clampedThis = *this;
	clampedThis.clamp();

	const float sPitch = std::sin(clampedThis._pitch);
	const float cPitch = std::cos(clampedThis._pitch);
	const float sYaw = std::sin(clampedThis._yaw);
	const float cYaw = std::cos(clampedThis._yaw);
	return vec3(cPitch * sYaw, sPitch, cPitch * cYaw);
}

vec3 rotatorRad::toRightDirection() const
{
	return rotatorRad(0.F, _yaw + 1.571F, 0.0F).toForwardVector();
}

void rotatorRad::clamp()
{
	constexpr float radMax = static_cast<float>(360.F * 180.F) / M_PI;
	_pitch = fmodf(_pitch, radMax);
	_yaw = fmodf(_yaw, radMax);
	_roll = fmodf(_roll, radMax);
}

rotatorDeg rotatorRad::toDegrees() const
{
	return rotatorDeg(static_cast<float>(_pitch * 180.F) / M_PI, static_cast<float>(_yaw * 180.F) / M_PI, static_cast<float>(_roll * 180.F) / M_PI);
}
