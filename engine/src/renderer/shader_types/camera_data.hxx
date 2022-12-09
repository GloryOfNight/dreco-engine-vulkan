#pragma once
#include "math/mat4.hxx"

struct camera_data
{
	de::math::mat4 view{de::math::mat4::makeIdentity()};
	de::math::mat4 viewProj{view * de::math::mat4::makeProjection(1.F, 10000.F, static_cast<float>(720) / static_cast<float>(720), 45.F)};
};