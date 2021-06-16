#pragma once

struct rotator 
{
    rotator() = default;

    rotator(const float pitch, const float yaw = 0.0F, const float roll = 0.0F);

    float _pitch;

    float _yaw;

    float _roll;
};