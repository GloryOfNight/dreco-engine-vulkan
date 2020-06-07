#include "engine.hxx"
#include "render/vulkan/vk_renderer.hxx"
#include <SDL2/SDL.h>
#include <iostream>

engine::engine() : _renderer{nullptr}
{
}

engine::~engine()
{
	if (_is_running)
		stop();
}

void engine::run()
{
	if (_is_running)
	{
		std::cerr << "Egnine already running, abording." << std::endl;
		return;
	}

	if (auto sdlInitResult{SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)}; 1 == sdlInitResult)
	{
		std::cout << "SDL Init error: " << SDL_GetError() << std::endl;
		throw std::runtime_error("SDL initialization failed. Cannot proceed.");
		return;
	}

	startRenderer();
	startMainLoop();
}

void engine::stop()
{
	if (false == _is_running)
	{
		std::cerr << "Engine aren't running, abording." << std::endl;
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
	_is_running = true;
	while (_is_running)
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
	_is_running = false;
}