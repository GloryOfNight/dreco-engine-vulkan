#pragma once

#include "camera.hxx"

class DRECO_API debug_camera final : public camera
{
public:
	debug_camera(world& w, root_node* owner = nullptr)
		: camera(w, owner)
	{
	}

	void tick(double deltaTime) override;

private:
	bool isMouseRightButtonRepeated = false;
};