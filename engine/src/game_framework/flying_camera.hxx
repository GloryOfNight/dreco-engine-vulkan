#pragma once

#include "camera.hxx"

namespace de::gf
{
	class DRECO_API flying_camera : public camera
	{
	public:
		void tick(double deltaTime) override;

	private:
		bool isMouseRightButtonRepeated = false;
	};
} // namespace de::gf