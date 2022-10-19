#include "gltf_model.hxx"

#include "core/engine.hxx"
#include "core/threads/async_tasks/async_load_gltf.hxx"

void gltf_model::init()
{
	node::init();

	auto task = engine::get()->getThreadPool().queueTask<async_load_gltf>(DRECO_ASSET(_modelPath));
	task->bindCallback(this, &gltf_model::onModelLoaded);
}

void gltf_model::onModelLoaded(thread_task* task)
{
	auto loadGltfTask = dynamic_cast<async_load_gltf*>(task);
	if (auto* eng = engine::get())
	{
		_model = loadGltfTask->extract();
		
		auto& renderer = eng->getRenderer();
		renderer.loadModel(_model);
	}
}