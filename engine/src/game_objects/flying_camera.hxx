#pragma once

#include "camera.hxx"

class DRECO_API flying_camera : public camera
{
public:
	void tick(double deltaTime) override;

private:
	bool isMouseRightButtonRepeated = false;
};