#include "game_instance.hxx"

#include "engine/engine.hxx"

#include "camera.hxx"

game_instance::game_instance(engine& eng)
	: _owner{eng}
{
}

void game_instance::init()
{
}

void game_instance::tick(double deltaTime)
{
	_currentWorld->tick(deltaTime);
}

camera const* game_instance::getActiveCamera() const
{
	return _activeCamera;
}

bool game_instance::setActiveCamera(camera& c)
{
	if (c.getWorld() == _currentWorld.get())
	{
		// what to do if camera was set active but deleted in process?
		_activeCamera = &c;
		return true;
	}
	return false;
}
