#pragma once
#include "core/containers/gltf/model.hxx"
#include "core/engine.hxx"
#include "core/loaders/gltf_loader.hxx"
#include "core/threads/thread_pool.hxx"

#include <string_view>

struct async_load_gltf : public thread_task
{
	using callback = std::function<void(const gltf::model&)>;

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

	gltf::model extract() { return std::move(_model); };

private:
	std::string _file;
	gltf::model _model;
};