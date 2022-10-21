#pragma once
#include "math/mat4.hxx"

struct camera_data
{
	mat4 view{mat4::makeIdentity()};
	mat4 viewProj{view * mat4::makeProjection(1.F, 10000.F, static_cast<float>(720) / static_cast<float>(720), 45.F)};
};