#include "engine/engine.hxx"

#include "core/objects/world.hxx"
#include "core/objects/game_instance.hxx"
#include "core/objects/debug_camera.hxx"


class launcher_world : public world
{
public:
	launcher_world(game_instance& gi)
		: world(gi)
	{
	}

	void init() override 
	{
		auto* cam = NewEntity<debug_camera>();
		getGameInstance()->setActiveCamera(*cam);
	}
};

class launcher_gi : public game_instance
{
public:
	launcher_gi(engine& eng)
		: game_instance(eng)
	{
	}
	void init() override 
	{
		loadWorld<launcher_world>();
	}
};


int main()
{
	engine Engine;
	const bool initRes = Engine.init();
	if (!initRes)
	{
		return 1;
	}

	Engine.runGI<launcher_gi>();
	return 0;
}