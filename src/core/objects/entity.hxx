#pragma once

#include "math/transform.hxx"

#include "dreco.hxx"

#include <memory>
#include <vector>

class world;
class DRECO_API entity
{
public:
	entity(world& w, entity* owner = nullptr);
	entity(entity&) = delete;
	entity(entity&&) = delete;

	virtual void init();

	virtual void tick(double deltaTime);

	transform _transform;

	template<class T>
	T* AddChild();

	world* getWorld() const { return &_world; };
	entity const* getOwner() const { return _owner; };

private:
	world& _world;

	entity* _owner;

	std::vector<std::unique_ptr<entity>> _children;
};

template <class T>
inline T* entity::AddChild()
{
	static_assert(std::is_base_of<T, entity>(), "T should be direved from entity");
	auto& newEntitity = _children.emplace_back(new T(_world, this));
	newEntitity->init();
	return dynamic_cast<T*>(newEntitity.get());
}
