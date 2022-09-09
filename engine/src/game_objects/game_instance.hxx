#pragma once
#include "dreco.hxx"
#include "world.hxx"

class engine;
class camera;

class DRECO_API game_instance
{
public:
	game_instance() = default;
	game_instance(game_instance&) = delete;
	game_instance(game_instance&&) = delete;

	virtual void init();

	template <class T>
	void loadWorld();

	virtual void tick(double deltaTime);

	const std::shared_ptr<camera>& getActiveCamera() const;
	bool setActiveCamera(const std::shared_ptr<camera>& cam);

	virtual std::unique_ptr<game_instance> makeNew() const = 0;

private:
	std::unique_ptr<world> _currentWorld;

	std::shared_ptr<camera> _activeCamera;
};

template <class T>
inline void game_instance::loadWorld()
{
	static_assert(std::is_base_of<world, T>(), "T should be direved from world");

	_currentWorld.reset(new T(*this));
	_currentWorld->init();
}