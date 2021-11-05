#include "debug_camera.hxx"

#include "engine/engine.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

void debug_camera::tick(double deltaTime)
{
	auto* engine = engine::get();
	const input_manager& inputManager = engine->getInputManager();

	auto transform = getTransform();
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
			pos += camFowVec * (camMoveSpeed * deltaTime);
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
		rotator& rotation = transform._rotation;

		vk_renderer* renderer = engine->getRenderer();
		SDL_Window* window = renderer->getWindow();
		if (SDL_GetMouseFocus() == window)
		{
			int x, y;
			const uint32_t mouseState = SDL_GetMouseState(&x, &y);
			if (mouseState == SDL_BUTTON_LMASK)
			{
				const auto extent = renderer->getCurrentExtent();
				if (isMouseRightButtonRepeated)
				{
					const double cameraRotSpeed = 1800.0;
					if (x)
					{
						const auto coefDistX = (static_cast<float>(extent.width / 2) / static_cast<float>(x) - 1);
						rotation._yaw = rotation._yaw - (cameraRotSpeed * coefDistX * deltaTime);
					}
					if (y)
					{
						const auto coefDistY = (static_cast<float>(extent.height / 2) / static_cast<float>(y) - 1);
						rotation._pitch = rotation._pitch + (cameraRotSpeed * coefDistY * deltaTime);
					}
				}
				else // on first button press
				{
					SDL_ShowCursor(SDL_DISABLE);
					isMouseRightButtonRepeated = true;
				}

				SDL_WarpMouseInWindow(window, static_cast<int>(extent.width) / 2, static_cast<int>(extent.height) / 2);
			}
			else if (isMouseRightButtonRepeated)
			{
				isMouseRightButtonRepeated = false;
				SDL_ShowCursor(SDL_ENABLE);
			}
		}
	}

	setTransform(transform);
	camera::tick(deltaTime);
}
