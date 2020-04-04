#pragma once

class vk_renderer;

class engine
{
public:
	engine();
	~engine();

	void runMainLoop();

private:
	vk_renderer* renderer;

	bool keep_main_loop = false;
};