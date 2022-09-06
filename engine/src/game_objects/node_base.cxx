#include "node_base.hxx"

node_base::node_base(world& w, node_base* owner)
	: _world(w)
	, _owner{owner}
{
}

void node_base::init()
{
}

void node_base::tick(double deltaTime)
{
	for (auto& child : _children)
		child->tick(deltaTime);
}
