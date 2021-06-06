#include "engine.hxx"

#include "core/loaders/gltf_loader.hxx"
#include "renderer/vulkan/vk_mesh.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

#include <SDL.h>
#include <chrono>
#include <iostream>

static inline engine* gEngine{nullptr};

engine::engine()
	: _renderer{nullptr}
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

void engine::run()
{
	if (_isRunning)
	{
		std::cerr << "Egnine already running, abording. \n";
		return;
	}

	if (gEngine != nullptr && gEngine->_isRunning)
	{
		std::cerr << "Another engine instance already running, abording. \n";
		return;
	}

	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 0 != sdlInitResult)
	{
		std::cerr << "SDL Initialization error: " << SDL_GetError() << "\n";
		return;
	}

	gEngine = this;

	if (true == startRenderer())
	{
		startMainLoop();
	}
}

void engine::stop()
{
	if (false == _isRunning)
	{
		std::cerr << "Engine aren't running, abording. \n";
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

			std::cout << "Vulkan Instance version: " << major << "." << minor << "." << patch << std::endl;

			std::vector<mesh_data> meshes_data = gltf_loader::loadScene("content/viking_room/scene.gltf");
			for (auto mesh : meshes_data)
			{
				auto newMesh = _renderer->createMesh(mesh);
				newMesh->_transform._rotation = vec3(3, 0, 0);
			}
		}
		else
		{
			std::cout << "Vulkan not supported by current driver or GPU." << std::endl;
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

void engine::startMainLoop()
{
	_isRunning = true;
	while (_isRunning)
	{
		const double deltaTime = calculateNewDeltaTime();
		if (deltaTime == 0.0)
		{
			continue; // skip tick if delta time zero
		}

		_renderer->tick(deltaTime);

		const float speed = 100.F;

		{ // rotating camera with mouse input
			if (SDL_GetMouseFocus() == _renderer->getWindow())
			{
				SDL_PumpEvents();
				static int mousePosX{0};
				static int mousePosY{0};
				int newMousePosX{0};
				int newMousePosY{0};
				const auto mouseState{SDL_GetMouseState(&newMousePosX, &newMousePosY)};

				if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					const float cofX = static_cast<float>(mousePosX) / static_cast<float>(newMousePosX) - 1;
					const float cofY = static_cast<float>(mousePosY) / static_cast<float>(newMousePosY) - 1;

					const vec3 camRot = _camera.getTransform()._rotation;
					const float camRotX = camRot._x + cofY * (speed * deltaTime);
					const float camRotY = camRot._y + cofX * (speed * deltaTime) * -1;

					_camera.setRotation(vec3(camRotX, camRotY, 0));
				}
				else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					const float cofY = static_cast<float>(mousePosY) / static_cast<float>(newMousePosY) - 1;
					const vec3 camPos = _camera.getTransform()._translation;
					const float camPosZ = (camPos._z + ((speed * 10) * deltaTime * cofY));
					_camera.setPosition(vec3(camPos._x, camPos._y, camPosZ));
				}
				else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE))
				{
					const float cofX = static_cast<float>(mousePosX) / static_cast<float>(newMousePosX) - 1;
					const float cofY = static_cast<float>(mousePosY) / static_cast<float>(newMousePosY) - 1;

					const vec3 camPos = _camera.getTransform()._translation;
					const float camPosX = camPos._x + ((speed * 10) * deltaTime * cofX);
					const float camPosY = camPos._y + (((speed * 10) * deltaTime * cofY) * -1);
					_camera.setPosition(vec3(camPosX, camPosY, camPos._z));
				}
				mousePosX = newMousePosX;
				mousePosY = newMousePosY;
			}
		}

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
					_camera.setPosition(cameraTranform._translation + vec3(0, 0, speed * deltaTime));
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					_camera.setPosition(cameraTranform._translation + vec3(0, 0, -speed * deltaTime));
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					_camera.setPosition(cameraTranform._translation + vec3(speed * deltaTime, 0, 0));
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					_camera.setPosition(cameraTranform._translation + vec3(-speed * deltaTime, 0, 0));
				}

				if (event.key.keysym.sym == SDLK_e)
				{
					_camera.setPosition(cameraTranform._translation + vec3(0, speed * deltaTime, 0));
				}
				else if (event.key.keysym.sym == SDLK_q)
				{
					_camera.setPosition(cameraTranform._translation + vec3(0, -speed * deltaTime, 0));
				}
			}
		}
	}
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