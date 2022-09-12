#include "world.hxx"

#include "root_node.hxx"

world::world(game_instance& gi)
	: _owner{gi}
{
}

void world::init()
{
}

void world::tick(double deltaTime)
{
	for (auto& ent : _nodes)
	{
		ent->tick(deltaTime);
	}
}
