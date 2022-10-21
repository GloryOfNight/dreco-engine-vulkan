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

void node::destroy()
{
	if (_owner)
		_owner->destroyChild(this);
	else
	{
		DE_LOG(Error, "%s: cannot destroy root node", __FUNCTION__);
	}
}

void node::destroyChild(node const* obj)
{
	if (obj && obj->_owner == this)
	{
		auto it = std::find_if(_children.begin(), _children.end(),
			[obj](auto& child)
			{
				return child.get() == obj;
			});
		_children.erase(it);
	}
	else
	{
		DE_LOG(Error, "%s: attempt to destroy invalid child ptr 0x%08x", __FUNCTION__, obj);
	}
}

bool node::apply()
{
	if (_world)
	{
		if (_owner && _owner->getWorld() != _world)
		{
			DE_LOG(Error, "%s: Failed to apply node. No world provided.", __FUNCTION__);
			return false;
		}
		node* parent = _owner != nullptr ? _owner : _world->getRootNode();
		if (parent)
			parent->_children.emplace(std::unique_ptr<node>(this));

		init();

		return true;
	}
	DE_LOG(Error, "%s: Failed to apply node. No world provided.", __FUNCTION__);
	return false;
}