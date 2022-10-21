#pragma once

#include "math/transform.hxx"

#include "dreco.hxx"

#include <memory>
#include <set>

class world;
class DRECO_API node
{
public:
	using unique = std::unique_ptr<node>;

	node() = default;
	node(node&) = delete;
	node(node&&) = delete;
	virtual ~node() = default;

	virtual void init();

	virtual void tick(double deltaTime);

	template <typename NodeClass, typename... Args>
	static NodeClass* newNode(world* inWorld, node* inOwner, Args&&... args);

	template <typename NodeClass, typename... Args>
	NodeClass* makeChild(world* inWorld, Args&&... args);

	void destroy();
	void destroyChild(node const* obj);

	world* getWorld() const { return _world; };
	node* getOwner() { return _owner; };

	transform& getTransform() { return _transform; };
	const transform& getTransform() const { return _transform; };

private:
	[[nodiscard]] bool apply();

	world* _world{};

	node* _owner{};

	transform _transform{};

	std::set<std::unique_ptr<node>> _children{};
};

template <typename NodeClass, typename... Args>
static NodeClass* newNode(world* inWorld, node* inOwner = nullptr, Args&&... args)
{
	return node::newNode<NodeClass>(inWorld, inOwner, std::forward<Args>(args)...);
}

template <typename NodeClass, typename... Args>
NodeClass* node::newNode(world* inWorld, node* inOwner, Args&&... args)
{
	static_assert(std::is_base_of<node, NodeClass>::value, "NodeClass should be direvied from node");
	if (inWorld == nullptr)
	{
		DE_LOG(Error, "%s: Cannot create %s. World is null.", __FUNCTION__, typeid(NodeClass).name());
		return nullptr;
	}

	auto newNode = new NodeClass(std::forward<Args>(args)...);
	newNode->_world = inWorld;
	newNode->_owner = inOwner != nullptr ? inOwner : inWorld->getRootNode();

	if (newNode->_owner == nullptr)
	{
		DE_LOG(Verbose, "%s: Creating root node %s.", __FUNCTION__, typeid(NodeClass).name());
	}
	else if (newNode->_owner->getWorld() != newNode->_world)
	{
		DE_LOG(Error, "%s: Cannot create %s. Owner world != this world.", __FUNCTION__, typeid(NodeClass).name());
		delete newNode;
		return nullptr;
	}
	else
	{
		newNode->_owner->_children.emplace(std::unique_ptr<node>(newNode));
	}

	newNode->init();
	return newNode;
}

template <typename NodeClass, typename... Args>
NodeClass* node::makeChild(world* inWorld, Args&&... args)
{
	return node::newNode<NodeClass>(inWorld, this, std::forward<Args>(args)...);
}