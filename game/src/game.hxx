#pragma once

#include "core/engine.hxx"
#include "game_framework/flying_camera.hxx"
#include "game_framework/game_instance.hxx"
#include "game_framework/gltf_model.hxx"
#include "game_framework/world.hxx"

class DRECO_API launcher_world : public de::gf::world
{
public:
	launcher_world(de::gf::game_instance& gi)
		: world(gi)
	{
	}

	void init() override
	{
		auto camera1 = de::gf::newNode<de::gf::flying_camera>(this, nullptr);
		const uint32_t view1Indx = de::engine::get()->addViewport("camera (1)");
		camera1->setViewId(view1Indx);

		//auto camera2 = de::gf::newNode<de::gf::flying_camera>(this);
		//const uint32_t view2Indx = de::engine::get()->addViewport("camera (2)");
		//camera2->setViewId(view2Indx);

		de::gf::newNode<de::gf::gltf_model>(this, nullptr, "mi-24d/scene.gltf");
		de::gf::newNode<de::gf::gltf_model>(this, nullptr, "viking_room/scene.gltf");
	}
};

class DRECO_API launcher_gi : public de::gf::game_instance
{
public:
	launcher_gi() = default;
	void init() override
	{
		loadWorld<launcher_world>();
	}
};