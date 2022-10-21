#pragma once

#include "game_objects/flying_camera.hxx"
#include "game_objects/gltf_model.hxx"
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
		newNode<flying_camera>(this);
		newNode<gltf_model>(this, nullptr, "mi-24d/scene.gltf");
		newNode<gltf_model>(this, nullptr, "viking_room/scene.gltf");
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

	virtual game_instance::unique makeNew() const override 
	{
		return game_instance::unique(new launcher_gi());
	}
};