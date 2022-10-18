#include "world.hxx"

#include "node.hxx"

world::world(game_instance& gi)
	: _owner{&gi}
	, _rootNode{}
{
	_rootNode = makeRootNode();
}

void world::init()
{
}

void world::tick(double deltaTime)
{
	_rootNode->tick(deltaTime);
}

node::unique world::makeRootNode()
{
	return node::unique(newNode<node>(this, nullptr));
}