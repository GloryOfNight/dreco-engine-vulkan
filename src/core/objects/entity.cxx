#include "entity.hxx"

entity::entity(world& w, entity* owner)
	: _world(w)
	, _owner{owner}
{
}

void entity::init()
{
}

void entity::tick(double deltaTime)
{
	for (auto& child : _children)
		child->tick(deltaTime);
}
