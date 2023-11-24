#pragma once

#include "math/transform.hxx"

#include "dreco.hxx"

#include <memory>
#include <set>

namespace de::gf
{
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

		void setName(std::string_view name) { _name = name; };
		std::string_view getName() const { return _name; };

		world* getWorld() const { return _world; };
		node* getOwner() { return _owner; };

		math::transform& getTransform() { return _transform; };
		const math::transform& getTransform() const { return _transform; };

	private:
		[[nodiscard]] bool apply(node* inOwner);

		std::string _name;

		world* _world{};

		node* _owner{};

		math::transform _transform{};

		std::set<std::unique_ptr<node>> _children{};
	};

	template <typename NodeClass, typename... Args>
	static NodeClass* newNode(world* inWorld, node* inOwner, Args&&... args)
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

		if (!newNode->apply(inOwner))
		{
			DE_LOG(Error, "%s: Failed to create %s.", __FUNCTION__, typeid(NodeClass).name());
			delete newNode;
			return nullptr;
		}

		newNode->init();
		return newNode;
	}

	template <typename NodeClass, typename... Args>
	NodeClass* node::makeChild(world* inWorld, Args&&... args)
	{
		return node::newNode<NodeClass>(inWorld, this, std::forward<Args>(args)...);
	}
} // namespace de::gf