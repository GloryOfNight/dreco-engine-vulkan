#include "debug_camera.hxx"

#include "engine/engine.hxx"

void debug_camera::tick(double deltaTime)
{
	const input_manager& inputManager = engine::get()->getInputManager();

	const float camMoveSpeed = 50.F;

	const auto& transform = getTransform();
	const vec3 camFowVec = transform._rotation.toForwardVector();
	const vec3 camRightVec = transform._rotation.toRightDirection();
	vec3 pos = transform._translation;

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

	setPosition(pos);

	camera::tick(deltaTime);
}
