#include "node.hxx"

#include "world.hxx"

void node::init()
{
}

void node::tick(double deltaTime)
{
	for (auto& child : _children)
		child->tick(deltaTime);
}

bool node::apply()
{
	if (_world)
	{
		if (_owner && _owner->getWorld() != _world)
		{
			return false;
		}
		node* parent = _owner != nullptr ? _owner : _world->getRootNode();
		if (parent)
			parent->_children.emplace_back(std::unique_ptr<node>(this));

		init();
		
		return true;
	}
	DE_LOG(Error, "%s: Failed to apply node. No world provided.", __FUNCTION__);
	return false;
}