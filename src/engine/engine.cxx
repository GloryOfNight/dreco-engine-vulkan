#include "engine.hxx"

#include "core/loaders/gltf_loader.hxx"
#include "core/utils/file_utils.hxx"
#include "core/utils/log.hxx"
#include "renderer/vulkan/vk_mesh.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

#include "engine.hxx"

#include <SDL.h>
#include <chrono>

static inline engine* gEngine{nullptr};

static void onQuitEvent(const SDL_Event&)
{
	auto engine = engine::get();
	if (engine)
		engine->stop();
}

struct async_task_load_scene : public thread_task
{
	async_task_load_scene(const std::string_view& sceneFile)
		: _file(sceneFile)
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

engine::engine()
	: _eventManager{}
	, _inputManager(_eventManager)
	, _threadPool("dreco-worker", thread_pool::hardwareConcurrency() / 2)
	, _renderer{}
	, _isRunning{false}
{
	_eventManager.addEventBinding(SDL_QUIT, &onQuitEvent);
}

engine::~engine()
{
	if (_isRunning)
	{
		stop();
	}
}

engine* engine::get()
{
	return gEngine;
}

const camera* engine::getCamera() const
{
	return &_camera;
}

bool engine::init()
{
	if (_isRunning)
	{
		DE_LOG(Error, "Egnine already running, cannot init.");
		return false;
	}

	if (gEngine != nullptr)
	{
		if (gEngine == this)
		{
			DE_LOG(Error, "This engine instance was already initialized.");
		}
		else
		{
			DE_LOG(Error, "Another engine instance already initialized.");
		}

		return false;
	}

	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 0 != sdlInitResult)
	{
		DE_LOG(Error, "SDL Initialization error: %s", SDL_GetError());
		return false;
	}

	if (!file_utils::isFileExists(TEXTURE_PLACEHOLDER_URI))
	{
		DE_LOG(Error, "Failed to find default texture: %s", TEXTURE_PLACEHOLDER_URI.c_str());
		DE_LOG(Error, "Current working directory: %s", file_utils::currentWorkingDir().data());
		return false;
	}

	gEngine = this;

	return true;
}

void engine::run()
{
	if (nullptr == engine::get())
	{
		DE_LOG(Error, "Init engine first. Cannot run.");
		return;
	}

	if (true == startRenderer())
	{
		startMainLoop();
	}
}

void engine::stop()
{
	if (false == _isRunning)
	{
		DE_LOG(Error, "Run engine first. Cannot stop.");
		return;
	}
	_isRunning = false;
}

bool engine::startRenderer()
{
	if (vk_renderer::isSupported())
	{
		_renderer.init();

		uint32_t major;
		uint32_t minor;
		uint32_t patch;
		_renderer.getVersion(major, minor, &patch);
		DE_LOG(Info, "Vulkan Instance version: %u.%u.%u", major, minor, patch);

		return true;
	}
	else
	{
		DE_LOG(Critical, "Vulkan not supported by current driver or GPU.");
	}

	return false;
}

void engine::preMainLoop()
{
	_threadPool.queueTask(new async_task_load_scene(DRECO_ASSET("mi-24d/scene.gltf")));

	_camera.setPosition(vec3(0, 10, 50));
	_camera.setRotation(rotator(0, 180, 0));
}

void engine::startMainLoop()
{
	preMainLoop();

	_isRunning = true;
	while (_isRunning)
	{
		const double deltaTime = calculateNewDeltaTime();
		if (deltaTime == 0.0)
		{
			continue; // skip tick if delta time zero
		}
		_eventManager.tick();
		_threadPool.tick();

		_camera.tick(deltaTime);
		_renderer.tick(deltaTime);
	}

	postMainLoop();
}

void engine::postMainLoop()
{
	_renderer.exit();
	SDL_Quit();
}

double engine::calculateNewDeltaTime()
{
	const auto frametime_from_fps_lam = [](const double fps) constexpr { return (1.0 / static_cast<double>(fps)); };
	constexpr double fpsMax = frametime_from_fps_lam(24000);
	constexpr double fpsMin = frametime_from_fps_lam(24);

	static std::chrono::time_point past = std::chrono::steady_clock::now();
	const std::chrono::time_point now = std::chrono::steady_clock::now();

	const auto microSeconds{std::chrono::duration_cast<std::chrono::microseconds>(now - past).count()};
	if (microSeconds == 0)
	{
		return 0.0;
	}

	const double delta{microSeconds / 1000000.0};
	if (delta < fpsMax)
	{
		return 0.0;
	}

	past = now;
	if (delta >= fpsMin)
	{
		return fpsMin;
	}

	return delta;
}