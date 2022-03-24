#pragma once

#include "camera.hxx"

class debug_camera final : public camera
{
public:
	debug_camera(world& w, entity* owner = nullptr) 
		: camera(w, owner)
	{
	}
	void tick(double deltaTime) override;

private:
	bool isMouseRightButtonRepeated = false;
};