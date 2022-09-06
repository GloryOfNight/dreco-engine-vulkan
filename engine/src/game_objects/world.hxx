#pragma once

#include "math/transform.hxx"

#include "node_base.hxx"

#include <memory>
#include <vector>

class game_instance;
class DRECO_API world
{
public:
	world(game_instance& gi);
	world(world&) = delete;
	world(world&&) = delete;

	virtual void init();

	virtual void tick(double deltaTime);

	template <class T>
	T* NewEntity();

	game_instance* getGameInstance() const { return &_owner; };

private:
	std::vector<std::unique_ptr<node_base>> _nodes;

	game_instance& _owner;
};

template <class T>
inline T* world::NewEntity()
{
	static_assert(std::is_base_of<node_base, T>(), "T should be direved from entity");
	if (this)
	{
		return dynamic_cast<T*>(_nodes.emplace_back(new T(*this)).get());
	}
	return nullptr;
}
