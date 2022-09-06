#pragma once

#include "game_objects/debug_camera.hxx"
#include "game_objects/game_instance.hxx"
#include "game_objects/world.hxx"

#include "dreco.hxx"

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

std::unique_ptr<game_instance> DRECO_API createGameInstance(engine& eng)
{
	return std::unique_ptr<game_instance>(new launcher_gi(eng));
}