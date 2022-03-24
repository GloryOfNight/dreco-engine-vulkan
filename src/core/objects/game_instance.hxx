#pragma once
#include "dreco.hxx"
#include "world.hxx"

class engine;
class camera;

class DRECO_API game_instance
{
public:
	game_instance(engine& eng);
	game_instance(game_instance&) = delete;
	game_instance(game_instance&&) = delete;

	virtual void init();

	template <class T>
	void loadWorld();

	virtual void tick(double deltaTime);

	camera const* getActiveCamera() const;
	bool setActiveCamera(camera& c);

private:
	engine& _owner;

	std::unique_ptr<world> _currentWorld;

	camera* _activeCamera;
};

template <class T>
inline void game_instance::loadWorld()
{
	static_assert(std::is_base_of<world, T>(), "T should be direved from world");
	if (this)
	{
		_currentWorld.reset(new T(*this));
		_currentWorld->init();
	}
}