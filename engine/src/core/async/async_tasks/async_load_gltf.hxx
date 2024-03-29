#pragma once
#include "core/engine.hxx"
#include "gltf/gltf.hxx"
#include "gltf/model.hxx"
#include "threads/thread_pool.hxx"

#include <string>

namespace de::async
{
	struct async_load_gltf : public thread_task
	{
		using callback = std::function<void(const de::gltf::model&)>;

		async_load_gltf(const std::string_view sceneUri)
			: _file(sceneUri)
		{
		}

		virtual void doJob() override
		{
			_model = de::gltf::loadModel(_file);
		}

		de::gltf::model extract() { return std::move(_model); };

	private:
		std::string _file;
		de::gltf::model _model;
	};
} // namespace de::async