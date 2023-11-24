#include "world.hxx"

#include "node.hxx"

de::gf::world::world(game_instance& gi)
	: _owner{&gi}
	, _rootNode{}
{
	_rootNode = makeRootNode();
}

void de::gf::world::init()
{
}

void de::gf::world::tick(double deltaTime)
{
	_rootNode->tick(deltaTime);
}

de::gf::node::unique de::gf::world::makeRootNode()
{
	return node::unique(newNode<node>(this, nullptr));
}