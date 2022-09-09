#include "game_instance.hxx"

#include "core/engine.hxx"

#include "camera.hxx"

void game_instance::init()
{
}

void game_instance::tick(double deltaTime)
{
	_currentWorld->tick(deltaTime);
}

const std::shared_ptr<camera>& game_instance::getActiveCamera() const
{
	return _activeCamera;
}

bool game_instance::setActiveCamera(const std::shared_ptr<camera>& cam)
{
	if (cam && cam->getWorld() == _currentWorld.get())
	{
		_activeCamera = cam;
		return true;
	}
	return false;
}
