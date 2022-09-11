#pragma once

#include "math/transform.hxx"

#include "dreco.hxx"
#include "world.hxx"

#include <memory>
#include <vector>

class DRECO_API node_base
{
public:
	node_base(world& w, node_base* owner = nullptr);
	node_base(node_base&) = delete;
	node_base(node_base&&) = delete;

	virtual void init();

	virtual void tick(double deltaTime);

	transform _transform;

	template <class T>
	T* AddChild();

	world& getWorld() const { return _world; };

	bool hasOwner() const { return !(_owner == nullptr); }
	node_base* getOwner() { return _owner; };

private:
	world& _world;

	node_base* _owner;

	std::vector<std::unique_ptr<node_base>> _children;
};

template <class T>
inline T* node_base::AddChild()
{
	static_assert(std::is_base_of<T, node_base>(), "T should be direved from node_base");
	auto& newEntitity = _children.emplace_back(new T(_world, this));
	newEntitity->init();
	return dynamic_cast<T*>(newEntitity.get());
}