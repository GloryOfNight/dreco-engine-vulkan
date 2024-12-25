#include "flying_camera.hxx"

#include "core/engine.hxx"
#include "math/casts.hxx"
#include "math/vec3.hxx"

void de::gf::flying_camera::init()
{
	camera::init();
	_inputManager.init(engine::get()->getEventManager());
}

void de::gf::flying_camera::tick(double deltaTime)
{
	auto* engine = de::engine::get();

	_inputManager.setWindowId(engine->getWindowId(getViewId()));

	auto& transform = getTransform();

	const auto quatRot = de::math::quat_cast(transform._rotation);
	const auto camFowVec = de::math::quaternion::forwardVector(quatRot);
	const auto camRightVec = de::math::quaternion::rightVector(quatRot);

	auto view = de::renderer::get()->getView(getViewId());
	if (view == nullptr)
	{
		// do not update camera if view is not present
		return;
	}

	{
		auto viewSettings = view->getSettings();
		if (_inputManager.isKeyPressed(SDLK_F1))
		{
			if (viewSettings.getPolygonMode() == vk::PolygonMode::eFill)
			{
				viewSettings.setPolygonMode(vk::PolygonMode::eLine);
			}
			else
			{
				viewSettings.setPolygonMode(vk::PolygonMode::eFill);
			}
		}

		if (_inputManager.isKeyPressed(SDLK_PAGEUP))
		{
			const auto sampleCount = viewSettings.getSampleCount();
			if (sampleCount < vk::SampleCountFlagBits::e64)
			{
				viewSettings.setSampleCount(static_cast<vk::SampleCountFlagBits>(static_cast<uint32_t>(sampleCount) * 2));
			}
		}
		else if (_inputManager.isKeyPressed(SDLK_PAGEDOWN))
		{
			const auto sampleCount = viewSettings.getSampleCount();
			if (sampleCount > vk::SampleCountFlagBits::e1)
			{
				viewSettings.setSampleCount(static_cast<vk::SampleCountFlagBits>(static_cast<uint32_t>(sampleCount) / 2));
			}
		}
		view->applySettings(std::move(viewSettings));
	}

	{ // camera WASDQE movement
		float camMoveSpeed = 50.F;
		if (_inputManager.isKeyPressed(SDLK_LSHIFT))
		{
			camMoveSpeed *= 2;
		}

		auto& pos = transform._translation;
		if (_inputManager.isKeyPressed(SDLK_W))
		{
			pos += camFowVec * camMoveSpeed * deltaTime;
		}
		else if (_inputManager.isKeyPressed(SDLK_S))
		{
			pos += camFowVec * (-camMoveSpeed * deltaTime);
		}

		if (_inputManager.isKeyPressed(SDLK_D))
		{
			pos += camRightVec * (camMoveSpeed * deltaTime);
		}
		else if (_inputManager.isKeyPressed(SDLK_A))
		{
			pos += camRightVec * (-camMoveSpeed * deltaTime);
		}

		if (_inputManager.isKeyPressed(SDLK_E))
		{
			pos += de::math::vec3(0, camMoveSpeed * deltaTime, 0);
		}
		else if (_inputManager.isKeyPressed(SDLK_Q))
		{
			pos += de::math::vec3(0, -camMoveSpeed * deltaTime, 0);
		}
	}

	{
		const auto& renderer = engine->getRenderer();

		if (_inputManager.isInMouseFocus())
		{
			uint16_t x, y;
			const uint32_t mouseState = _inputManager.getMouseState(&x, &y);

			if (mouseState == SDL_BUTTON_LMASK)
			{
				int w, h;
				SDL_GetWindowSize(de::engine::get()->getWindow(getViewId()), &w, &h);

				const auto halfExtentX = static_cast<uint32_t>(w * 0.5f);
				const auto halfExtentY = static_cast<uint32_t>(h * 0.5f);
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
					_inputManager.setMouseRelativeMode(true);
					isMouseRightButtonRepeated = true;
				}
				_inputManager.warpMouse(halfExtentX, halfExtentY);
			}
			else if (isMouseRightButtonRepeated)
			{
				_inputManager.setMouseRelativeMode(false);
				isMouseRightButtonRepeated = false;
			}
		}
		transform._rotation.clamp();
		transform._rotation.max(de::math::Pi * 0.5f, de::math::Pi * 2.f, de::math::Pi * 2.f);
		transform._rotation.min(-de::math::Pi * 0.5f, -de::math::Pi * 2.f, -de::math::Pi * 2.f);
	}

	camera::tick(deltaTime);
}
