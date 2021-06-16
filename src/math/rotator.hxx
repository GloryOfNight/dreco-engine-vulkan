#pragma once

struct vec3;

struct rotator
{
	rotator() = default;

	rotator(const float pitch, const float yaw = 0.0F, const float roll = 0.0F);

	vec3 toForwardVector() const;

	vec3 toRightDirection() const;

	rotator toRadians() const;

	void clamp();

	float _pitch;

	float _yaw;

	float _roll;
};

rotator operator+(const rotator& first, const rotator& second);
rotator operator-(const rotator& first, const rotator& second);