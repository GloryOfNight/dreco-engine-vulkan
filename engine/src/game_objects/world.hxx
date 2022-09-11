#pragma once

#include "math/transform.hxx"

#include <memory>
#include <vector>

class game_instance;
class node_base;
class DRECO_API world
{
public:
	world(game_instance& gi);
	world(world&) = delete;
	world(world&&) = delete;

	virtual void init();

	virtual void tick(double deltaTime);

	template <class T>
	std::shared_ptr<T> NewEntity();

	game_instance& getGameInstance() const { return _owner; };

private:
	std::vector<std::shared_ptr<node_base>> _nodes;

	game_instance& _owner;
};

template <class T>
inline std::shared_ptr<T> world::NewEntity()
{
	static_assert(std::is_base_of<node_base, T>(), "T should be direved from entity");

	auto newObj = std::shared_ptr<T>(new T(*this));
	_nodes.emplace_back(newObj);
	return newObj;
}
