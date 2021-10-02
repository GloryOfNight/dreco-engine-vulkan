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

struct async_task_load_scene : public thread_task
{
	async_task_load_scene(const std::string_view& sceneFile)
		: _file(sceneFile)
	{
	}

	virtual void init() override{};

	virtual void doJob() override
	{
		_scene = gltf_loader::loadScene(_file);
	};

	virtual void completed() override
	{
		if (auto* eng = engine::get())
		{
			if (auto* renderer = eng->getRenderer())
			{
				renderer->loadScene(_scene);
			}
		}
	};

private:
	std::string _file;

	scene _scene;
};

engine::engine()
	: _threadPool{nullptr}
	, _renderer{nullptr}
	, _isRunning{false}
{
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

vk_renderer* engine::getRenderer() const
{
	return _renderer;
}

const camera* engine::getCamera() const
{
	return &_camera;
}

thread_pool* engine::getThreadPool() const
{
	return _threadPool;
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

	stopMainLoop();
	stopRenderer();
	SDL_Quit();
}

bool engine::startRenderer()
{
	if (_renderer == nullptr)
	{
		if (vk_renderer::isSupported())
		{
			_renderer = new vk_renderer();
			_renderer->init();

			uint32_t major;
			uint32_t minor;
			uint32_t patch;
			_renderer->getVersion(major, minor, &patch);
			DE_LOG(Info, "Vulkan Instance version: %u.%u.%u", major, minor, patch);
		}
		else
		{
			DE_LOG(Critical, "Vulkan not supported by current driver or GPU.");
		}
	}
	return _renderer != nullptr;
}

void engine::stopRenderer()
{
	if (_renderer)
	{
		delete _renderer;
		_renderer = nullptr;
	}
}

void engine::preMainLoop()
{
	if (_threadPool == nullptr)
	{
		_threadPool = new thread_pool("dreco-worker", thread_pool::hardwareConcurrency() / 2);
	}

	_threadPool->queueTask(new async_task_load_scene(DRECO_ASSET("viking_room/scene.gltf")));

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
		_threadPool->tick();

		_renderer->tick(deltaTime);

		const float camMoveSpeed = 100.F;
		const float camRotSpeed = 1800.F;

		{ // rotating camera with mouse input
			if (SDL_GetMouseFocus() == _renderer->getWindow())
			{
				SDL_PumpEvents();
				static int mousePosX{0};
				static int mousePosY{0};
				int newMousePosX{0};
				int newMousePosY{0};
				const auto mouseState{SDL_GetMouseState(&newMousePosX, &newMousePosY)};

				int windowSizeX{0};
				int windowSizeY{0};
				SDL_GetWindowSize(_renderer->getWindow(), &windowSizeX, &windowSizeY);

				const float cofX = static_cast<float>(mousePosX) / static_cast<float>(newMousePosX) - static_cast<float>(windowSizeX / windowSizeY);
				const float cofY = static_cast<float>(mousePosY) / static_cast<float>(newMousePosY) - 1;

				const bool isCoefValid = !((std::isnan(cofX) || std::isinf(cofX)) || (std::isnan(cofY) || std::isinf(cofY)));
				if (isCoefValid)
				{
					if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
					{
						const rotator camRot = _camera.getTransform()._rotation;
						const float camRotX = camRot._pitch + cofY * (camRotSpeed * deltaTime);
						const float camRotY = camRot._yaw + cofX * (camRotSpeed * deltaTime) * -1;

						_camera.setRotation(rotator(camRotX, camRotY, 0));
					}
					else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
					{
						const vec3 camPos = _camera.getTransform()._translation;
						const float camPosZ = (camPos._z + ((camRotSpeed * 10) * deltaTime * cofY));
						_camera.setPosition(vec3(camPos._x, camPos._y, camPosZ));
					}
					else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE))
					{
						const vec3 camPos = _camera.getTransform()._translation;
						const float camPosX = camPos._x + ((camRotSpeed * 10) * deltaTime * cofX);
						const float camPosY = camPos._y + (((camRotSpeed * 10) * deltaTime * cofY) * -1);
						_camera.setPosition(vec3(camPosX, camPosY, camPos._z));
					}
					mousePosX = newMousePosX;
					mousePosY = newMousePosY;
				}
			}
		}

		const vec3 camFowVec = _camera.getTransform()._rotation.toForwardVector();
		const vec3 camRightVec = _camera.getTransform()._rotation.toRightDirection();

		// event poll not working very well with high frame rate (>60)
		// TODO: input state machine, should do job just fine
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				stop();
			}
			else if (event.type == SDL_KEYDOWN)
			{
				const transform cameraTranform = _camera.getTransform();

				if (event.key.keysym.sym == SDLK_w)
				{
					_camera.setPosition(cameraTranform._translation + (camFowVec * (camMoveSpeed * deltaTime)));
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					_camera.setPosition(cameraTranform._translation + (camFowVec * (-camMoveSpeed * deltaTime)));
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					_camera.setPosition(cameraTranform._translation + camRightVec * (camMoveSpeed * deltaTime));
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					_camera.setPosition(cameraTranform._translation + camRightVec * (-camMoveSpeed * deltaTime));
				}

				if (event.key.keysym.sym == SDLK_e)
				{
					_camera.setPosition(cameraTranform._translation + vec3(0, camMoveSpeed * deltaTime, 0));
				}
				else if (event.key.keysym.sym == SDLK_q)
				{
					_camera.setPosition(cameraTranform._translation + vec3(0, -camMoveSpeed * deltaTime, 0));
				}

				if (event.key.keysym.sym == SDLK_MINUS)
				{
					auto& settings = _renderer->getSettings();
					if (settings.setPrefferedSampleCount(static_cast<VkSampleCountFlagBits>(settings.getPrefferedSampleCount() / 2)))
					{
						_renderer->applySettings();
					}
				}
				else if (event.key.keysym.sym == SDLK_EQUALS)
				{
					auto& settings = _renderer->getSettings();
					if (settings.setPrefferedSampleCount(static_cast<VkSampleCountFlagBits>(settings.getPrefferedSampleCount() * 2)))
					{
						_renderer->applySettings();
					}
				}
			}
		}
	}

	postMainLoop();
}

void engine::postMainLoop()
{
	delete _threadPool;
	_threadPool = nullptr;
}

void engine::stopMainLoop()
{
	_isRunning = false;
}

double engine::calculateNewDeltaTime()
{
#define FRAMETIME_FROM_FPS(FPS) (1.0 / static_cast<double>(FPS))
	constexpr double FPS_MAX = FRAMETIME_FROM_FPS(60);
	constexpr double FPS_MIN = FRAMETIME_FROM_FPS(24);

	static std::chrono::time_point past = std::chrono::steady_clock::now();
	const std::chrono::time_point now = std::chrono::steady_clock::now();

	const auto microSeconds{std::chrono::duration_cast<std::chrono::microseconds>(now - past).count()};
	if (microSeconds == 0)
	{
		return 0.0;
	}

	const double delta{microSeconds / 1000000.0};
	if (delta < FPS_MAX)
	{
		return 0.0;
	}

	past = now;
	if (delta >= FPS_MIN)
	{
		return FPS_MIN;
	}

	return delta;
}