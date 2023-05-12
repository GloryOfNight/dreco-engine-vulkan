#pragma once

#include "game_framework/flying_camera.hxx"
#include "game_framework/gltf_model.hxx"
#include "game_framework/game_instance.hxx"
#include "game_framework/world.hxx"

#include "dreco.hxx"

class launcher_world : public de::gf::world
{
public:
	launcher_world(de::gf::game_instance& gi)
		: world(gi)
	{
	}

	void init() override
	{
		de::gf::newNode<de::gf::flying_camera>(this);
		de::gf::newNode<de::gf::gltf_model>(this, nullptr, "mi-24d/scene.gltf");
		de::gf::newNode<de::gf::gltf_model>(this, nullptr, "viking_room/scene.gltf");
	}
};

class launcher_gi : public de::gf::game_instance
{
public:
	launcher_gi() = default;
	void init() override
	{
		loadWorld<launcher_world>();
	}
};