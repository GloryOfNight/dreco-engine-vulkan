#pragma once

#include "math/transform.hxx"

#include "dreco.hxx"
#include "world.hxx"

#include <memory>
#include <vector>

class DRECO_API root_node
{
public:
	root_node(world& w, root_node* owner = nullptr);
	root_node(root_node&) = delete;
	root_node(root_node&&) = delete;
	virtual ~root_node() = default;

	virtual void init();

	virtual void tick(double deltaTime);

	transform _transform;

	template <class T>
	T* AddChild();

	world& getWorld() const { return _world; };

	bool hasOwner() const { return !(_owner == nullptr); }
	root_node* getOwner() { return _owner; };

private:
	world& _world;

	root_node* _owner;

	std::vector<std::unique_ptr<root_node>> _children;
};

template <class T>
inline T* root_node::AddChild()
{
	static_assert(std::is_base_of<T, root_node>(), "T should be direved from node_base");
	auto& newEntitity = _children.emplace_back(new T(_world, this));
	newEntitity->init();
	return dynamic_cast<T*>(newEntitity.get());
}
