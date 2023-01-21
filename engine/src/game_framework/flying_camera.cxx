#include "flying_camera.hxx"

#include "core/engine.hxx"
#include "math/casts.hxx"
#include "math/vec3.hxx"

void de::gf::flying_camera::tick(double deltaTime)
{
	auto* engine = de::engine::get();
	auto& inputManager = engine->getInputManager();

	auto& transform = getTransform();

	const auto quatRot = de::math::quat_cast(transform._rotation);
	const auto camFowVec = de::math::quaternion::forwardVector(quatRot);
	const auto camRightVec = de::math::quaternion::rightVector(quatRot);

	{ // camera WASDQE movement
		float camMoveSpeed = 50.F;
		if (inputManager.isKeyPressed(SDLK_LSHIFT))
		{
			camMoveSpeed *= 2;
		}

		auto& pos = transform._translation;
		if (inputManager.isKeyPressed(SDLK_w))
		{
			pos += camFowVec * camMoveSpeed * deltaTime;
		}
		else if (inputManager.isKeyPressed(SDLK_s))
		{
			pos += camFowVec * (-camMoveSpeed * deltaTime);
		}

		if (inputManager.isKeyPressed(SDLK_d))
		{
			pos += camRightVec * (camMoveSpeed * deltaTime);
		}
		else if (inputManager.isKeyPressed(SDLK_a))
		{
			pos += camRightVec * (-camMoveSpeed * deltaTime);
		}

		if (inputManager.isKeyPressed(SDLK_e))
		{
			pos += de::math::vec3(0, camMoveSpeed * deltaTime, 0);
		}
		else if (inputManager.isKeyPressed(SDLK_q))
		{
			pos += de::math::vec3(0, -camMoveSpeed * deltaTime, 0);
		}
	}

	{
		const auto& renderer = engine->getRenderer();

		if (inputManager.isInMouseFocus())
		{
			uint16_t x, y;
			const uint32_t mouseState = inputManager.getMouseState(&x, &y);

			if (mouseState == SDL_BUTTON_LMASK)
			{
				const auto extent = renderer.getCurrentExtent();
				const auto halfExtentX = static_cast<uint32_t>(extent.width * 0.5f);
				const auto halfExtentY = static_cast<uint32_t>(extent.height * 0.5f);
				if (isMouseRightButtonRepeated)
				{
					const auto cameraRotSpeed = de::math::deg_to_rad(45.f);
					if (x && halfExtentX != x)
					{
						const auto coefDistX = (static_cast<float>(halfExtentX) / static_cast<float>(x) - 1);
						transform._rotation._yaw = transform._rotation._yaw + (cameraRotSpeed * coefDistX);
					}
					if (y && halfExtentY != y)
					{
						const auto coefDistY = (static_cast<float>(halfExtentY) / static_cast<float>(y) - 1);
						transform._rotation._pitch = transform._rotation._pitch + (cameraRotSpeed * coefDistY);
					}
				}
				else // on first button press
				{
					inputManager.setMouseRelativeMode(true);
					isMouseRightButtonRepeated = true;
				}
				inputManager.warpMouse(halfExtentX, halfExtentY);
			}
			else if (isMouseRightButtonRepeated)
			{
				inputManager.setMouseRelativeMode(false);
				isMouseRightButtonRepeated = false;
			}
		}
		transform._rotation.clamp();
		transform._rotation.max(M_PI * 0.5f, M_PI * 2.f, M_PI * 2.f);
		transform._rotation.min(-M_PI * 0.5f, -M_PI * 2.f, -M_PI * 2.f);
	}

	camera::tick(deltaTime);
}
