#include "node.hxx"

#include "world.hxx"

#include <algorithm>

void de::gf::node::init()
{
}

void de::gf::node::tick(double deltaTime)
{
	for (auto& child : _children)
		child->tick(deltaTime);
}

void de::gf::node::destroy()
{
	if (_owner)
		_owner->destroyChild(this);
	else
	{
		DE_LOG(Error, "%s: cannot destroy root node", __FUNCTION__);
	}
}

void de::gf::node::destroyChild(node const* obj)
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

bool de::gf::node::apply(node* inOwner)
{
	_owner = inOwner != nullptr ? inOwner : _world->getRootNode();
	if (_owner != nullptr && _owner->getWorld() != _world)
	{
		DE_LOG(Error, "%s: Failed to apply node, owner world != this world.", __FUNCTION__);
		return false;
	}

	node* parent = _owner != nullptr ? _owner : _world->getRootNode();
	if (parent)
		parent->_children.emplace(std::unique_ptr<node>(this));

	return true;
}