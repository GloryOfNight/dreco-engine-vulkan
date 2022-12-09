#include "game_instance.hxx"

#include "core/engine.hxx"

#include "camera.hxx"
#include "world.hxx"

void de::gf::game_instance::init()
{
}

void de::gf::game_instance::tick(double deltaTime)
{
	for (auto& world : _worlds)
	{
		world->tick(deltaTime);
	}
}

const std::vector<de::gf::world::unique>& de::gf::game_instance::getWorlds() const
{
	return _worlds;
}

de::gf::world& de::gf::game_instance::getCurrentWorld() const
{
	return *_worlds[_currentWorldIndex];
}

bool de::gf::game_instance::setCurrentWorldIndex(const size_t index)
{
	if (_worlds.size() <= index)
	{
		_currentWorldIndex = index;
		return true;
	}
	return false;
}
