#include "engine.hxx"

#include "renderer/vulkan/vk_mesh.hxx"
#include "renderer/vulkan/vk_renderer.hxx"
#include "load_scene.hxx"

#include <SDL.h>
#include <iostream>

static inline engine* gEngine{nullptr};

engine::engine()
	: _renderer{nullptr}
	, isRunning{false}
	, lastTickTime{0}
{
}

engine::~engine()
{
	if (isRunning)
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

void engine::run()
{
	if (isRunning)
	{
		std::cerr << "Egnine already running, abording. \n";
		return;
	}

	if (gEngine != nullptr && gEngine->isRunning)
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
	if (false == isRunning)
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

			mesh_data mesh_data = loadScene("content/viking_room/scene.gltf");

			auto mesh = _renderer->createMesh(mesh_data);
			mesh->_transform._translation = vec3(0, 6, 0);
			mesh->_transform._rotation = vec3(3, 0, 0);
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
	lastTickTime = SDL_GetPerformanceCounter();

	isRunning = true;
	while (isRunning)
	{
		double DeltaTime{0};
		calculateNewDeltaTime(DeltaTime);

		_renderer->tick(DeltaTime);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				stop();
			}
			//else if (event.type == SDL_KEYDOWN)
			//{
			//	const float speed = 100.F;
			//	if (event.key.keysym.sym == SDLK_w)
			//	{
			//	}
			//	else if (event.key.keysym.sym == SDLK_s)
			//	{
			//	}

			//	if (event.key.keysym.sym == SDLK_d)
			//	{
			//	}
			//	else if (event.key.keysym.sym == SDLK_a)
			//	{
			//	}

			//	if (event.key.keysym.sym == SDLK_e)
			//	{
			//	}
			//	else if (event.key.keysym.sym == SDLK_q)
			//	{
			//	}
			//}
		}
	}
}

void engine::stopMainLoop()
{
	isRunning = false;
}

void engine::calculateNewDeltaTime(double& NewDeltaTime)
{
	const uint64_t now{SDL_GetPerformanceCounter()};

	NewDeltaTime = static_cast<double>(now - lastTickTime) / static_cast<double>(SDL_GetPerformanceFrequency());

	lastTickTime = now;
}