#include "engine.hxx"
#include "render/vulkan/vk_renderer.hxx"
#include <SDL2/SDL.h>
#include <iostream>

engine::engine() : _renderer{nullptr}
{
}

engine::~engine()
{
	if (isRunning)
	{
		stop();
	}
}

void engine::run()
{
	if (isRunning)
	{
		std::cerr << "Egnine already running, abording. \n";
		return;
	}

	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 0 != sdlInitResult)
	{
		std::cerr << "SDL Initialization error: " << SDL_GetError() << "\n";
		throw std::runtime_error("SDL initialization failed. Cannot proceed.");
		return;
	}

	startRenderer();
	startMainLoop();
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

void engine::startRenderer()
{
	if (_renderer == nullptr)
	{
		_renderer = new vk_renderer(this);
	}
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
		_renderer->tick(0.0f);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				stop();
			}
		}
	}
}

void engine::stopMainLoop()
{
	isRunning = false;
}