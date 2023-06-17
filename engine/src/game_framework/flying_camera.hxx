#pragma once

#include "camera.hxx"

#include "core/managers/input_manager.hxx"

namespace de::gf
{
	class DRECO_API flying_camera : public camera
	{
	public:
		void init() override;

		void tick(double deltaTime) override;

	private:
		bool isMouseRightButtonRepeated = false;

		input_manager _inputManager;
	};
} // namespace de::gf