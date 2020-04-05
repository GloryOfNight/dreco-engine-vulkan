#include "engine.hxx"

#include "render/vulkan/vk_renderer.hxx"

#include <GLFW/glfw3.h>
#include <iostream>

engine::engine()
{
	glfwInit();

	if (vk_renderer::isSupported())
	{
		renderer = new vk_renderer(this);
		renderer->drawFrame();
	}
	else
	{
		std::cerr << "Vulkan is not supported, abording. " << std::endl;
		return;
	}
}

engine::~engine()
{
	if (renderer)
		delete renderer;
	glfwTerminate();
}

void engine::runMainLoop()
{
	keep_main_loop = true;
	while (keep_main_loop)
	{
		renderer->tick(0.0f);
		glfwPollEvents();
	
		int state = glfwGetKey(renderer->getWindow(), GLFW_KEY_ESCAPE);
		if (state == GLFW_PRESS)
		{
			keep_main_loop = false;
		}
	}
}