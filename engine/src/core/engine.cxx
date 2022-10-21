#include "engine.hxx"

#include "core/loaders/gltf_loader.hxx"
#include "core/threads/async_tasks/async_load_gltf.hxx"
#include "core/utils/file_utils.hxx"
#include "renderer/vk_mesh.hxx"
#include "renderer/vk_renderer.hxx"

#include "engine.hxx"

#include <SDL.h>
#include <chrono>
#include <csignal>
#include <shader_compiler.hxx>

static inline engine* gEngine{nullptr};

static void onQuitEvent(const SDL_Event&)
{
	auto engine = engine::get();
	if (engine)
		engine->stop();
}

engine::engine()
	: _eventManager{}
	, _inputManager(_eventManager)
	, _threadPool("dreco-worker", thread_pool::hardwareConcurrency() / 2)
	, _renderer{}
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

engine::init_res engine::initialize()
{
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

		return init_res::AlreadyInitialized;
	}

	if (_isRunning)
	{
		DE_LOG(Error, "Egnine already running, cannot init.");
		return init_res::AlreadyRunning;
	}

	registerSignals();
	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 0 != sdlInitResult)
	{
		DE_LOG(Error, "SDL Initialization error: %s", SDL_GetError());
		return init_res::FailedInitSDL;
	}

	if (!platform_paths::init())
	{
		DE_LOG(Error, "Failed to locate proper Cwd, current working dir: %s", platform_paths::currentDir().c_str());
		return init_res::FailedFindCWD;
	}

	gEngine = this;

	return init_res::Ok;
}

engine::run_res engine::run()
{
	if (nullptr == engine::get())
	{
		DE_LOG(Error, "Init engine first. Cannot run.");
		return run_res::Unitialized;
	}

	if (!_defaultGameInstance.isSet())
	{
		DE_LOG(Error, "Game Instance isn't registred.");
		return run_res::InvalidGameInstance;
	}

	_gameInstance = _defaultGameInstance.makeNew();
	if (nullptr == _gameInstance)
	{
		DE_LOG(Error, "Game Instance object nullptr, coundn't run");
		return run_res::FailedMakeNewGameInstance;
	}

	if (true == startRenderer())
	{
		startMainLoop();
	}
	return run_res::Ok;
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

void engine::registerSignals()
{
	std::signal(SIGINT, engine::onSystemSignal);
	std::signal(SIGILL, engine::onSystemSignal);
	std::signal(SIGFPE, engine::onSystemSignal);
	std::signal(SIGSEGV, engine::onSystemSignal);
	std::signal(SIGTERM, engine::onSystemSignal);
	std::signal(SIGABRT, engine::onSystemSignal);
}

void engine::onSystemSignal(int sig)
{
	if (sig == SIGFPE)
		DE_LOG(Info, "Engine recieved floating point excetion. . . Stopping engine.");
	else if (sig == SIGSEGV)
		DE_LOG(Info, "Engine recieved segment violation. . . Stopping engine.");
	else if (sig == SIGABRT || sig == SIGTERM || sig == SIGINT)
		DE_LOG(Info, "Engine stop signal. . . Stopping engine.");

	auto* eng = engine::get();
	if (eng)
		eng->stop();
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
	_gameInstance->init();
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
		++_frameCounter;

		_eventManager.tick();
		_threadPool.tick(_frameCounter);

		_gameInstance->tick(deltaTime);

		_renderer.tick(deltaTime);
	}

	postMainLoop();
}

void engine::postMainLoop()
{
	_renderer.exit();
	_gameInstance.reset();
	SDL_Quit();
}

double engine::calculateNewDeltaTime() noexcept
{
	const auto frametime_from_fps_lam = [](const double fps) constexpr noexcept
	{
		return (1.0 / static_cast<double>(fps));
	};
	constexpr double ftMax = frametime_from_fps_lam(5000);
	constexpr double ftMin = frametime_from_fps_lam(24);

	static std::chrono::time_point past = std::chrono::steady_clock::now();
	const std::chrono::time_point now = std::chrono::steady_clock::now();

	const auto microSeconds{std::chrono::duration_cast<std::chrono::microseconds>(now - past).count()};
	if (microSeconds == 0)
	{
		return 0.0;
	}

	const double delta{microSeconds / 1000000.0};
	if (delta < ftMax)
	{
		return 0.0;
	}

	past = now;
	if (delta >= ftMin)
	{
		return ftMin;
	}

	return delta;
}