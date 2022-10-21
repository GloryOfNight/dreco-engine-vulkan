#include "game_instance.hxx"

#include "core/engine.hxx"

#include "camera.hxx"
#include "world.hxx"

void game_instance::init()
{
}

void game_instance::tick(double deltaTime)
{
	for (auto& world : _worlds)
	{
		world->tick(deltaTime);
	}
}

const std::vector<world::unique>& game_instance::getWorlds() const
{
	return _worlds;
}

world& game_instance::getCurrentWorld() const
{
	return *_worlds[_currentWorldIndex];
}

bool game_instance::setCurrentWorldIndex(const size_t index)
{
	if (_worlds.size() <= index)
	{
		_currentWorldIndex = index;
		return true;
	}
	return false;
}
