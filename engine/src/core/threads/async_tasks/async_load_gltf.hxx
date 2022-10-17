#pragma once
#include "core/containers/gltf/model.hxx"
#include "core/engine.hxx"
#include "core/threads/thread_pool.hxx"

#include <string_view>

struct async_load_gltf : public thread_task
{
	template <typename Str>
	async_load_gltf(Str&& sceneUri)
		: _file(sceneUri)
	{
	}

	virtual void init() override{};

	virtual void doJob() override
	{
		_model = gltf_loader::loadModel(_file);
	}

	virtual void completed() override
	{
		if (auto* eng = engine::get())
		{
			auto& renderer = eng->getRenderer();
			renderer.loadModel(_model);
		}
	}

private:
	std::string _file;

	gltf::model _model;
};