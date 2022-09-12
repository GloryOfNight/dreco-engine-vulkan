#include "root_node.hxx"

root_node::root_node(world& w, root_node* owner)
	: _world(w)
	, _owner{owner}
{
}

void root_node::init()
{
}

void root_node::tick(double deltaTime)
{
	for (auto& child : _children)
		child->tick(deltaTime);
}
