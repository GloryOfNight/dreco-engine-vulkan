#pragma once
#include "dreco.hxx"

#include "world.hxx"

#include <memory>
#include <vector>

class camera;
class DRECO_API game_instance
{
public:
	game_instance() = default;
	game_instance(game_instance&) = delete;
	game_instance(game_instance&&) = delete;
	virtual ~game_instance() = default;

	virtual void init();

	template <class T>
	size_t loadWorld(const bool makeCurrent = true);

	virtual void tick(double deltaTime);

	const std::vector<std::unique_ptr<world>>& getWorlds() const;
	world& getCurrentWorld() const;
	bool setCurrentWorldIndex(const size_t index);

	virtual std::unique_ptr<game_instance> makeNew() const = 0;

private:
	std::vector<std::unique_ptr<world>> _worlds;
	size_t _currentWorldIndex{};
};

template <class T>
inline size_t game_instance::loadWorld(const bool makeCurrent)
{
	static_assert(std::is_base_of<world, T>(), "T should be direved from world");

	_worlds.push_back(std::unique_ptr<world>(new T(*this)));
	const size_t newWorldIndex = _worlds.size() - 1;

	_worlds[newWorldIndex]->init();

	if (makeCurrent)
		_currentWorldIndex = newWorldIndex;

	return newWorldIndex;
}