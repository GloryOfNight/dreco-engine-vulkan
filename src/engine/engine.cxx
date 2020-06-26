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
			else if(event.type == SDL_KEYDOWN) 
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