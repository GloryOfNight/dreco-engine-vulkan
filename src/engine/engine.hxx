#pragma once

class vk_renderer;

class engine
{
public:
	engine();
	~engine();

	void run();

	void stop();

private:
	void startRenderer();

	void stopRenderer();
	
	void startMainLoop();	
	
	void stopMainLoop();

	vk_renderer* _renderer;

	bool _is_running = false;

	bool _keep_main_loop = false;
};