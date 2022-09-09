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
		getGameInstance()->setActiveCamera(NewEntity<debug_camera>());
	}
};

class launcher_gi : public game_instance
{
public:
	launcher_gi() = default;
	void init() override
	{
		loadWorld<launcher_world>();
	}

	virtual std::unique_ptr<game_instance> makeNew() const override 
	{
		return std::unique_ptr<game_instance>(new launcher_gi());
	}
};