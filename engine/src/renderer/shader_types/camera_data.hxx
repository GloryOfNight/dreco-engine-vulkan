#pragma once
#include "math/mat4.hxx"

struct camera_data
{
	de::math::mat4 view{de::math::mat4::makeIdentity()};
	de::math::mat4 proj{de::math::mat4::makeIdentity()};
};