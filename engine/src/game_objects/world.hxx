#pragma once

#include "math/transform.hxx"

#include "node.hxx"

#include <memory>

class game_instance;
class DRECO_API world
{
public:
	using unique = std::unique_ptr<world>;

	world(game_instance& gi);
	world(world&) = delete;
	world(world&&) = delete;
	virtual ~world() = default;

	virtual void init();

	virtual void tick(double deltaTime);

	virtual node::unique makeRootNode();

	game_instance* getGameInstance() const { return _owner; };

	node* getRootNode() const { return _rootNode.get(); };

private:
	game_instance* _owner;

	node::unique  _rootNode;
};