#include "engine.hxx"

#include "renderer/vulkan/vk_renderer.hxx"

#include <SDL.h>
#include <iostream>

static inline engine* gEngine{nullptr};

engine::engine()
	: _renderer{nullptr}
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

			_renderer->createMesh();
			_renderer->createMesh();
			_renderer->createMesh();
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
	isRunning = true;
	while (isRunning)
	{
		float DeltaTime;
		calculateNewDeltaTime(DeltaTime);

		_renderer->tick(DeltaTime);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				stop();
			}
			else if (event.type == SDL_KEYDOWN)
			{
				const float speed = 100.f;
				if (event.key.keysym.sym == SDLK_w)
				{
					shapeTranslation._y += DeltaTime * speed;
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					shapeTranslation._y -= DeltaTime * speed;
				}

				if (event.key.keysym.sym == SDLK_d)
				{
					shapeTranslation._x += DeltaTime * speed;
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					shapeTranslation._x -= DeltaTime * speed;
				}

				if (event.key.keysym.sym == SDLK_e)
				{
					shapeTranslation._z += DeltaTime * speed;
				}
				else if (event.key.keysym.sym == SDLK_q)
				{
					shapeTranslation._z -= DeltaTime * speed;
				}
			}
		}
	}
}

void engine::stopMainLoop()
{
	isRunning = false;
}

void engine::calculateNewDeltaTime(float& NewDeltaTime)
{
	const uint64_t now{SDL_GetPerformanceCounter()};

	NewDeltaTime = static_cast<float>(now - lastTickTime) / static_cast<float>(SDL_GetPerformanceFrequency());

	lastTickTime = now;
}