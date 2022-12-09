#include "gltf_model.hxx"

#include "core/async/async_tasks/async_load_gltf.hxx"
#include "core/engine.hxx"

void de::gf::gltf_model::init()
{
	node::init();

	auto task = de::engine::get()->getThreadPool().queueTask<de::async::async_load_gltf>(DRECO_ASSET(_modelPath));
	task->bindCallback(this, &gltf_model::onModelLoaded);
}

void de::gf::gltf_model::onModelLoaded(de::async::thread_task* task)
{
	auto loadGltfTask = dynamic_cast<de::async::async_load_gltf*>(task);
	if (auto* eng = de::engine::get())
	{
		_model = loadGltfTask->extract();

		auto& renderer = eng->getRenderer();
		renderer.loadModel(_model);
	}
}