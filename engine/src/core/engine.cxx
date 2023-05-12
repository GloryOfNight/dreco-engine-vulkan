#include "engine.hxx"

#include "core/async/async_tasks/async_load_gltf.hxx"
#include "core/containers/gltf/gltf.hxx"
#include "core/misc/file.hxx"
#include "shader_compiler_tool/shader_compiler.hxx"

#include "engine.hxx"

#include <SDL.h>
#include <chrono>
#include <csignal>

static inline de::engine* gEngine{nullptr};

static void onQuitEvent(const SDL_Event&)
{
	auto engine = de::engine::get();
	if (engine)
		engine->stop();
}

de::engine::engine()
	: _eventManager{}
	, _inputManager(_eventManager)
	, _threadPool()
	, _renderer{}
{
	_eventManager.addEventBinding(SDL_EVENT_QUIT, &onQuitEvent);
}

de::engine::~engine()
{
	if (_isRunning)
	{
		stop();
	}
}

de::engine* de::engine::get()
{
	return gEngine;
}

void de::engine::initialize()
{
	if (gEngine != nullptr)
	{
		if (gEngine == this)
		{
			DE_LOG(Error, "%s: this engine instance was already initialized.", __FUNCTION__);
		}
		else
		{
			DE_LOG(Error, "%s: another engine instance already initialized.", __FUNCTION__);
		}

		throw de::except::initialization_error();
		return;
	}

	if (_isRunning)
	{
		DE_LOG(Error, "%s: egnine already running, cannot init.", __FUNCTION__);
		throw de::except::initialization_error();
		return;
	}

	registerSignals();
	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 0 != sdlInitResult)
	{
		DE_LOG(Error, "%s: sdl initialization error: %s", __FUNCTION__, SDL_GetError());
		throw de::except::initialization_error();
		return;
	}

	if (!de::platform::path::init())
	{
		DE_LOG(Error, "%s: failed to locate proper cwd, current working dir: %s", __FUNCTION__, de::platform::path::currentDir().c_str());
		throw de::except::initialization_error();
		return;
	}

	gEngine = this;
}

void de::engine::run()
{
	if (nullptr == de::engine::get())
	{
		DE_LOG(Error, "%s: engine unitilized.", __FUNCTION__);
		return;
	}

	try
	{
		_gameInstance = _createGameInstanceFunc();
	}
	catch (std::bad_function_call)
	{
		DE_LOG(Error, "%s: __createGameInstance bad call, coundn't run", __FUNCTION__);
		return;
	}

	if (nullptr == _gameInstance)
	{
		DE_LOG(Error, "%s: game instance == nullptr, coundn't run", __FUNCTION__);
		return;
	}

	_threadPool.allocateThreads("dreco-engine-worker (low)", 2, de::async::thread_pool::priority::low);

	if (true == startRenderer())
	{
		startMainLoop();
	}
}

void de::engine::setCreateGameInstanceFunc(std::function<de::gf::game_instance::unique()> func)
{
	_createGameInstanceFunc = func;
}

void de::engine::stop()
{
	if (false == _isRunning)
	{
		DE_LOG(Error, "%s: engine not running, nothing to stop.", __FUNCTION__);
		return;
	}
	_isRunning = false;
	_threadPool.freeThreads();
}

void de::engine::registerSignals()
{
	std::signal(SIGINT, engine::onSystemSignal);
	std::signal(SIGILL, engine::onSystemSignal);
	std::signal(SIGFPE, engine::onSystemSignal);
	std::signal(SIGSEGV, engine::onSystemSignal);
	std::signal(SIGTERM, engine::onSystemSignal);
	std::signal(SIGABRT, engine::onSystemSignal);
}

void de::engine::onSystemSignal(int sig)
{
	if (sig == SIGFPE)
		DE_LOG(Info, "%s: engine recieved floating point excetion. . . stopping.", __FUNCTION__);
	else if (sig == SIGSEGV)
		DE_LOG(Info, "%s: engine recieved segment violation. . . stopping.", __FUNCTION__);
	else if (sig == SIGABRT || sig == SIGTERM || sig == SIGINT)
		DE_LOG(Info, "%s: engine stop signal. . . stopping.", __FUNCTION__);

	auto* eng = engine::get();
	if (eng)
		eng->stop();
}

bool de::engine::startRenderer()
{
	if (de::vulkan::renderer::isSupported())
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

void de::engine::preMainLoop()
{
	_gameInstance->init();
}

void de::engine::startMainLoop()
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
		_fpsCounter.tick(deltaTime);

		_eventManager.tick();
		_threadPool.tick(_frameCounter);

		_gameInstance->tick(deltaTime);

		_renderer.tick(deltaTime);
	}

	postMainLoop();
}

void de::engine::postMainLoop()
{
	_renderer.exit();
	_gameInstance.reset();
	SDL_Quit();
}

double de::engine::calculateNewDeltaTime() noexcept
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