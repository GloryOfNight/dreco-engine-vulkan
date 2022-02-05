#pragma once
#include "vec.hxx"

template <typename T>
struct irotatorT
{
	irotatorT() = default;
	irotatorT(const T pitch, const T yaw, const T roll)
		: _pitch{pitch}
		, _yaw{yaw}
		, _roll{roll}
	{
	}

	virtual vec3 toForwardVector() const = 0;

	virtual vec3 toRightDirection() const = 0;

	virtual void clamp() = 0;
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

using irotator_base = irotatorT<float>;
struct rotatorDeg;
struct rotatorRad;

struct DRECO_API rotatorDeg : irotator_base
{
	rotatorDeg() = default;
	rotatorDeg(const float pitch, const float yaw, const float roll)
		: irotator_base(pitch, yaw, roll)
	{
	}

	vec3 toForwardVector() const override;

	vec3 toRightDirection() const override;

	void clamp() override;

	rotatorRad toRadians() const;
};

struct DRECO_API rotatorRad : irotator_base
{
	rotatorRad() = default;
	rotatorRad(const float pitch, const float yaw, const float roll)
		: irotator_base(pitch, yaw, roll)
	{
	}

	vec3 toForwardVector() const override;

	vec3 toRightDirection() const override;

	void clamp() override;

	rotatorDeg toDegrees() const;
};