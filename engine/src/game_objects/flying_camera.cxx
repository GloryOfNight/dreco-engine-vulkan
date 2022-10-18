#include "flying_camera.hxx"

#include "math/vec3.hxx"

#include "core/utils/log.hxx"
#include "core/engine.hxx"
#include "renderer/vk_renderer.hxx"

void flying_camera::tick(double deltaTime)
{
	auto* engine = engine::get();
	input_manager& inputManager = engine->getInputManager();

	auto& transform = getTransform();
	const vec3 camFowVec = transform._rotation.toForwardVector();
	const vec3 camRightVec = transform._rotation.toRightDirection();

	{ // camera WASDQE movement
		float camMoveSpeed = 50.F;
		if (inputManager.isKeyPressed(SDLK_LSHIFT))
		{
			camMoveSpeed *= 2;
		}

		vec3& pos = transform._translation;
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
			pos += vec3(0, camMoveSpeed * deltaTime, 0);
		}
		else if (inputManager.isKeyPressed(SDLK_q))
		{
			pos += vec3(0, -camMoveSpeed * deltaTime, 0);
		}
	}

	{
		rotatorDeg& rotation = transform._rotation;

		const vk_renderer& renderer = engine->getRenderer();
		SDL_Window* window = renderer.getWindow();

		if (inputManager.isInMouseFocus())
		{
			uint16_t x, y;
			const uint32_t mouseState = inputManager.getMouseState(&x, &y);

			if (mouseState == SDL_BUTTON_LMASK)
			{
				const auto extent = renderer.getCurrentExtent();
				const int halfExtentX = extent.width * 0.5;
				const int halfExtentY = extent.height * 0.5;
				if (isMouseRightButtonRepeated)
				{
					const double cameraRotSpeed = 45.0;
					if (x && halfExtentX != x)
					{
						const auto coefDistX = (static_cast<float>(halfExtentX) / static_cast<float>(x) - 1);
						rotation._yaw = rotation._yaw - (cameraRotSpeed * coefDistX);
					}
					if (y && halfExtentY != y)
					{
						const auto coefDistY = (static_cast<float>(halfExtentY) / static_cast<float>(y) - 1);
						rotation._pitch = rotation._pitch + (cameraRotSpeed * coefDistY);
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
		rotation.clamp();
		rotation.max(90.F, 360.F, 360.F);
		rotation.min(-90.F, -360.F, -360.F);
	}

	camera::tick(deltaTime);
}
