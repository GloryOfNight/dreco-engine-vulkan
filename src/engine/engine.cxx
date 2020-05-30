#include "engine.hxx"
#include "render/vulkan/vk_renderer.hxx"
#include <GLFW/glfw3.h>
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

	if (auto glfwInitResult{glfwInit()}; GLFW_FALSE == glfwInitResult)
	{
		std::runtime_error("GLFW initialization failed. Cannot proceed.");
		return;
	}

	startRenderer();
	startMainLoop();

	_is_running = true;
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
	glfwTerminate();
}

void engine::startRenderer()
{
	if (false == vk_renderer::isSupported())
	{
		std::runtime_error("Vulkan is not supported. Cannot proceed.");
		return;
	}

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
	_keep_main_loop = true;
	while (_keep_main_loop)
	{
		_renderer->tick(0.0f);
		glfwPollEvents();

		if (glfwWindowShouldClose(_renderer->getWindow()))
		{
			stop();
		}
	}
}

void engine::stopMainLoop()
{
	_keep_main_loop = false;
}