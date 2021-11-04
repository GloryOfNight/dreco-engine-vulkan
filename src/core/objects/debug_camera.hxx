#pragma once

#include "camera.hxx"

class debug_camera final : public camera
{
public:
	void tick(double deltaTime) override;
};